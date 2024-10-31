from flask import Flask, request, jsonify
import json
from datetime import datetime as dt
import requests
import threading

import logging
import sys

logger = logging.getLogger()
logger.setLevel(logging.INFO)
formatter = logging.Formatter('%(asctime)s | %(levelname)s | %(message)s')

stdout_handler = logging.StreamHandler(sys.stdout)
stdout_handler.setLevel(logging.DEBUG)
stdout_handler.setFormatter(formatter)

file_handler = logging.FileHandler('sync_log.txt')
file_handler.setLevel(logging.DEBUG)
file_handler.setFormatter(formatter)

logger.addHandler(file_handler)
logger.addHandler(stdout_handler)

service_port_global = 37020
service_port_local = 510
timeToKillPublicSession = 7200 # 2 hours
timeToKillTestSession = 30 # test session - 30 seconds

lastTimeCheck = dt.now()

activeSessionTokens = {}

app = Flask(__name__)


@app.route('/api', methods=['POST'])
def api():
    print(request.data)
    logger.info(request.data)
    reqData = request.data.decode(encoding='cp1251', errors='ignore')
    if request.method == 'POST':
        #reqJson = request.get_json()
        reqJson = json.loads(reqData, strict=False)
        method = str(reqJson["method"])
        data = reqJson["data"]

        if method == 'checkConnection':
            response = {
                'error': '',
                'data': True
            }
            return jsonify(response)

        if method == 'getDataUsers':
            reqJson = json.loads(request.data.decode(encoding='utf-8', errors='ignore'), strict=False) # for some reason encoding is not valid here
            data = reqJson["data"]
            sessionToken = reqJson["sessionToken"]
            if sessionToken in activeSessionTokens: # if session exists - request or load usersTalents
                tloc = threading.local()
                session = activeSessionTokens[sessionToken]
                if 'usersData' in session:
                    return session['usersData']
                r = requests.post('https://playpw.fun/api/launcher/', json={"method": "getDataUsers", "data": data})
                activeSessionTokens[sessionToken]['usersData'] = r.content
                return r.content
            else:
                return "No session exists"


        if method == "registerUserInSession":
            def checkOldSessions():
                #fast check available sessions to kill
                for oldSessionToken in list(activeSessionTokens.keys()):
                    oldSession = activeSessionTokens[oldSessionToken]
                    targetTimeDifferenceToKill = timeToKillPublicSession
                    if oldSessionToken == 'testSessionToken':
                        targetTimeDifferenceToKill = timeToKillTestSession
                    if (dt.now() - oldSession['timestamp']).total_seconds() > targetTimeDifferenceToKill:
                        del activeSessionTokens[oldSessionToken] # remove old sessions

            checkOldSessions()
            
            sessionToken = data["sessionToken"]
            if sessionToken not in activeSessionTokens:
                tloc = threading.local()
                if sessionToken not in activeSessionTokens:
                    activeSessionTokens[sessionToken] = {'token': sessionToken, 'gameName': '', 'players': [ { 'nickname': data["nickname"], 'heroId': data["heroId"] } ], 'timestamp': dt.now()}
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
                        if player['nickname'] == data["nickname"]: # player already registered in this session
                            response = {
                                'error': 'PlayerAlreadyRegistered',
                                'data': ''
                            }
                            return jsonify(response)
                    activeSessionTokens[sessionToken]['players'].append({ 'nickname': data["nickname"], 'heroId': data["heroId"] })
                    response = {
                        'error': '',
                        'data': session['gameName']
                    }
                    return jsonify(response)
            else: # if session does not exist - create one
                activeSessionTokens[sessionToken] = {'token': sessionToken, 'gameName': '', 'players': [ { 'nickname': data["nickname"], 'heroId': data["heroId"] } ], 'timestamp': dt.now()}
                response = {
                    'error': '',
                    'data': ''
                }
                return jsonify(response)

        if method == "lobbyCreated":
            sessionToken = data["sessionToken"]
            if sessionToken in activeSessionTokens:
                session = activeSessionTokens[sessionToken]

                if session['gameName'] == '':  # save gameId
                    activeSessionTokens[sessionToken]['gameName'] = data["nickname"] # just save creator's nickname for now
                    response = {
                        'error': '',
                        'data': 0
                    }
                    return jsonify(response)
                else:
                    return 'Game already registered by someone else'
            else:
                return 'Invalid session id'

        if method == "checkIsGameReady":
            sessionToken = data["sessionToken"]
            if sessionToken in activeSessionTokens:
                session = activeSessionTokens[sessionToken]

                logger.info(sessionToken + ' players ready: ' + str(len(session['players'])))
                if len(session['players']) < 10:  # not enough players
                    response = {
                        'error': '',
                        'data': False
                    }
                    return jsonify(response)
                else:
                    response = {
                        'error': '',
                        'data': True
                    }
                    return jsonify(response)
            else:
                return 'Invalid session id'

        if method == "getGameNameForReconnect":
            sessionToken = data["sessionToken"]
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

if __name__ == '__main__':
    app.run(debug=True, port=service_port_global + service_port_local, host="127.0.0.1", threaded=True)
    