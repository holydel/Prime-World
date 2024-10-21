from flask import Flask, request, jsonify
import json

service_port_global = 35010
service_port_local = 500

activeSessionTokens = {}

#INVALID_GAME_ID = -1

app = Flask(__name__)


@app.route('/api', methods=['POST'])
def api():
    data = request.data.decode('cp1251')
    if request.method == 'POST':
        #reqJson = request.get_json()
        reqJson = json.loads(data)
        method = str(reqJson["method"])
        data = reqJson["data"]

        if method == "registerUserInSession":
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
                    activeSessionTokens[sessionToken]['players'].append({ 'nickname': data["nickname"], 'heroId': data["heroId"] })
                    response = {
                        'error': '',
                        'data': session['gameName']
                    }
                    return jsonify(response)
            else: # if session does not exist - create one
                activeSessionTokens[sessionToken] = {'gameName': '', 'players': [ { 'nickname': data["nickname"], 'heroId': data["heroId"] } ]}
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
    app.run(debug=True, port=service_port_global + service_port_local)