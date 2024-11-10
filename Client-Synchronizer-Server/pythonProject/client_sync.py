from flask import Flask, request, jsonify
import json
from datetime import datetime as dt
import requests
import threading
import hashlib

import logging
import sys

from requests import session

logger = logging.getLogger()
logger.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s | %(levelname)s | %(message)s')

stdout_handler = logging.StreamHandler(sys.stdout)
stdout_handler.setLevel(logging.DEBUG)
stdout_handler.setFormatter(formatter)

file_handler = logging.FileHandler('sync_log.txt',encoding='utf-8')
file_handler.setLevel(logging.DEBUG)
file_handler.setFormatter(formatter)


logger.addHandler(file_handler)
logger.addHandler(stdout_handler)

api_key = open('api_key.txt').readline()

timeToKillPublicSession = 7200 # 2 hours
timeToKillTestSession = 30 # test session - 30 seconds

lastTimeCheck = dt.now()

activeSessionTokens = {}

webSessions = {}

app = Flask(__name__)

def killOldSessions(sessionDict):
    for oldSessionToken in list(sessionDict.keys()):
        oldSession = sessionDict[oldSessionToken]
        targetTimeDifferenceToKill = timeToKillPublicSession
        if oldSessionToken == 'testSessionToken':
            targetTimeDifferenceToKill = timeToKillTestSession
        if (dt.now() - oldSession['timestamp']).total_seconds() > targetTimeDifferenceToKill:
            del sessionDict[oldSessionToken] # remove old sessions

@app.route('/api', methods=['POST'])
def api():
    print(request.data)
    logger.info(request.data)
    reqData = request.data.decode(encoding='cp1251', errors='replace')
    if request.method == 'POST':
        reqJson = json.loads(reqData, strict=False)
        method = str(reqJson['method'])
        data = 0
        if 'data' in reqJson:
            data = reqJson['data']
        if 'body' in reqJson:
            data = reqJson['body']

        # Register session (from web-launcher)
        if method == 'registerSession':
            killOldSessions(webSessions)

            key = reqJson['key']
            if key != api_key:
                response = {
                    'error': 'Invalid API key'
                }
                return jsonify(response)

            if data['sessionToken'] in webSessions:
                response = {
                    'error': 'Session already registered'
                }
                return jsonify(response)

            if not data['players']:
                response = {
                    'error': 'Failed to get players list'
                }
                return jsonify(response)

            webSessions[data['sessionToken']] = {'players': {}, 'lock': threading.Lock(), 'timestamp': dt.now(), 'gameName': '', 'gameStarted': False, 'gameFinished': False }

            # generate player keys, fill player data
            for player in data['players']:
                if 'id' not in player:
                    response = {
                        'error': 'Invalid player detected (no id) in session ' + data['sessionToken']
                    }
                    return jsonify(response)

                m = hashlib.sha256((str(player['id']) + data['sessionToken'] + api_key).encode('utf-8'))
                webSessions[data['sessionToken']]['players'][m.hexdigest()] = player
                logger.info(m.hexdigest())

            response = {
                'error': ''
            }
            return jsonify(response)

        # Connect player to web-session
        if method == 'connectToWebSession':
            if 'sessionToken' not in data or 'playerKey' not in data:
                response = {
                    'error': 'Invalid request'
                }
                return jsonify(response)

            sessionToken = data['sessionToken']
            playerKey = data['playerKey']

            if sessionToken not in webSessions:
                response = {
                    'error': 'Session token was not found in active sessions'
                }
                return jsonify(response)

            if playerKey not in webSessions[sessionToken]['players']:
                response = {
                    'error': 'Invalid player key'
                }
                return jsonify(response)

            # critical section for specific session!
            with webSessions[sessionToken]['lock']:
                method = ''
                if not webSessions[sessionToken]['gameName']:
                    method = 'create'
                else:
                    if webSessions[sessionToken]['gameStarted']:
                        method = 'reconnect'
                    else:
                        method = 'connect'

                response = {
                    'error': '',
                    'method': method,
                    'playerInfo': webSessions[sessionToken]['players'][playerKey],
                    'usersData': list(webSessions[sessionToken]['players'].values()),
                    'gameName': webSessions[sessionToken]['gameName']
                }
                return jsonify(response)

        # Notify game start event
        if method == 'notifyGameStart':
            if 'sessionToken' not in data:
                response = {
                    'error': 'Invalid request'
                }
                return jsonify(response)
            sessionToken = data['sessionToken']

            with webSessions[sessionToken]['lock']:
                webSessions[sessionToken]['gameStarted'] = True
                response = {
                    'error': '',
                }
                return jsonify(response)

        if method == 'notifyGameFinish':
            if 'sessionToken' not in data:
                response = {
                    'error': 'Invalid request'
                }
                return jsonify(response)
            sessionToken = data['sessionToken']

            with webSessions[sessionToken]['lock']:
                if webSessions[sessionToken]['gameFinished']:
                    response = {
                        'error': 'Game finished'
                    }
                    return jsonify(response)
                else:
                    webSessions[sessionToken]['gameFinished'] = True
                    requests.post('https://playpw.fun/api/launcher/', json={'method': 'finishGame', 'data': data})
                    response = {
                        'error': '',
                    }
                    return jsonify(response)

        # Get game name if is in connection process
        if method == 'getGameNameForConnection':
            if 'sessionToken' not in data:
                response = {
                    'error': 'Invalid request'
                }
                return jsonify(response)
            sessionToken = data['sessionToken']

            with webSessions[sessionToken]['lock']:
                if webSessions[sessionToken]['gameStarted']:
                    response = {
                        'error': 'reconnect',
                        'gameName': webSessions[sessionToken]['gameName']
                    }
                    return jsonify(response)
                response = {
                    'error': '',
                    'gameName': webSessions[sessionToken]['gameName']
                }
                return jsonify(response)

        # Web-Lobby created by player
        if method == 'webLobbyCreated':
            if 'sessionToken' not in data:
                response = {
                    'error': 'Invalid request'
                }
                return jsonify(response)
            sessionToken = data['sessionToken']
            if sessionToken in webSessions:
                with webSessions[sessionToken]['lock']:
                    webSession = webSessions[sessionToken]

                    if webSession['gameName'] == '':  # save gameId
                        webSessions[sessionToken]['gameName'] = data['nickname']
                        response = {
                            'error': ''
                        }
                        return jsonify(response)
                    else:
                        return 'Game already registered by someone else'
            else:
                return 'Invalid session id'

        # Check connection (checkInstall)
        if method == 'checkConnection':
            response = {
                'error': '',
                'data': True
            }
            return jsonify(response)

        # (Deprecated) Get Users Data
        if method == 'getDataUsers':
            reqJson = json.loads(request.data.decode(encoding='utf-8', errors='ignore'), strict=False) # for some reason encoding is not valid here
            data = reqJson['data']
            sessionToken = reqJson['sessionToken']
            if sessionToken in activeSessionTokens: # if session exists - request or load usersTalents
                session = activeSessionTokens[sessionToken]
                if 'usersData' in session:
                    return session['usersData']
                r = requests.post('https://playpw.fun/api/launcher/', json={'method': 'getDataUsers', 'data': data})
                activeSessionTokens[sessionToken]['usersData'] = r.content
                return r.content
            else:
                return 'No session exists'

        # (Deprecated) Register user in (custom) session
        if method == 'registerUserInSession':
            killOldSessions(activeSessionTokens)
            
            sessionToken = data['sessionToken']
            if sessionToken not in activeSessionTokens:
                if sessionToken not in activeSessionTokens:
                    activeSessionTokens[sessionToken] = {'token': sessionToken, 'gameName': '', 'players': [ { 'nickname': data['nickname'], 'heroId': data['heroId'] } ], 'timestamp': dt.now()}
                    response = {
                        'error': '',
                        'data': ''
                    }
                    return jsonify(response)
                
            if sessionToken in activeSessionTokens: # if session exists - try to add new player
                session = activeSessionTokens[sessionToken]

                if session['gameName'] == '': # wait for creator
                    response = {
                        'error': 'Wait',
                        'data': ''
                    }
                    return jsonify(response)
                else: # lobby was successfully created
                    for player in activeSessionTokens[sessionToken]['players']:
                        if player['nickname'] == data['nickname']: # player already registered in this session
                            response = {
                                'error': 'PlayerAlreadyRegistered',
                                'data': ''
                            }
                            return jsonify(response)
                    activeSessionTokens[sessionToken]['players'].append({ 'nickname': data['nickname'], 'heroId': data['heroId'] })
                    response = {
                        'error': '',
                        'data': session['gameName']
                    }
                    return jsonify(response)
            else: # if session does not exist - create one
                activeSessionTokens[sessionToken] = {'token': sessionToken, 'gameName': '', 'players': [ { 'nickname': data['nickname'], 'heroId': data['heroId'] } ], 'timestamp': dt.now()}
                response = {
                    'error': '',
                    'data': ''
                }
                return jsonify(response)

        # (Deprecated) Lobby created by player
        if method == 'lobbyCreated':
            sessionToken = data['sessionToken']
            if sessionToken in activeSessionTokens:
                session = activeSessionTokens[sessionToken]

                if session['gameName'] == '':  # save gameId
                    activeSessionTokens[sessionToken]['gameName'] = data['nickname'] # just save creator's nickname for now
                    response = {
                        'error': '',
                        'data': 0
                    }
                    return jsonify(response)
                else:
                    return 'Game already registered by someone else'
            else:
                return 'Invalid session id'

        # (Deprecated) Get game name for reconnect
        if method == 'getGameNameForReconnect':
            sessionToken = data['sessionToken']
            if sessionToken in activeSessionTokens:
                session = activeSessionTokens[sessionToken]
                
                if 'usersData' not in session:
                    activeSessionTokens[sessionToken]['players'].append({ 'nickname': 'FakeReconnect', 'heroId': 'FakeHero' })
                    response = {
                        'error': 'Connect',
                        'data': session['gameName']
                    }
                    return jsonify(response)

                response = {
                    'error': '',
                    'data': session['gameName']
                }
                return jsonify(response)
            else:
                return 'Invalid session id'

        return 'Unknown method in json'
    else:
        return 'Request method is not allowed'
