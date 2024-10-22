from flask import Flask, request, jsonify
import json
from datetime import datetime as dt
import requests

service_port_global = 35000
service_port_local = 510
timeToKillPublicSession = 7200 # 2 hours
timeToKillTestSession = 30 # test session - 30 seconds

timeToCheckOldSessions = 30 # better be lower or equal than test session kill time

lastTimeCheck = dt.now()

activeSessionTokens = {}

app = Flask(__name__)


@app.route('/api', methods=['POST'])
def api():
    data = request.data.decode('cp1251')
    if request.method == 'POST':
        #reqJson = request.get_json()
        reqJson = json.loads(data)
        method = str(reqJson["method"])
        data = reqJson["data"]

        if method == 'checkConnection':
            response = {
                'error': '',
                'data': True
            }
            return jsonify(response)

        if method == 'getDataUsers':
            sessionToken = reqJson["sessionToken"]
            if sessionToken in activeSessionTokens: # if session exists - request or load usersTalents
                session = activeSessionTokens[sessionToken]
                if 'usersData' in session:
                    return session['usersData']
                r = requests.post('https://playpw.fun/api/launcher/', json={"method": "getDataUsers", "data": data})
                activeSessionTokens[sessionToken]['usersData'] = r
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

            sessionToken = data["sessionToken"]
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
                            checkOldSessions()
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
                checkOldSessions()
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

        return 'Unknown method in json'
    else:
        return 'Request method is not allowed'

if __name__ == '__main__':
    app.run(debug=True, port=service_port_global + service_port_local, host="0.0.0.0")