from flask import Flask, request, jsonify

service_port_global = 35010
service_port_local = 500

activeSessionTokens = {}

INVALID_GAME_ID = -1

app = Flask(__name__)

@app.route('/api', methods=['POST'])
def api():
    if request.method == 'POST':
        reqJson = request.get_json()
        method = str(reqJson["method"])
        data = reqJson["data"]
        if method == "registerUserInSession":
            sessionToken = data["sessionToken"]
            if sessionToken in activeSessionTokens: # if session exists - try to add new player
                session = activeSessionTokens[sessionToken]

                if session['gameId'] == INVALID_GAME_ID: # wait for creator
                    response = {
                        'error': 'Wait',
                        'data': ''
                    }
                    return jsonify(response)
                else: # lobby successfully was created
                    activeSessionTokens[sessionToken]['players'].append({ 'nickname': data["nickname"], 'heroId': data["heroId"] })
                    response = {
                        'error': '',
                        'data': session['gameId']
                    }
                    return jsonify(response)
            else: # if session does not exist - create one
                activeSessionTokens[sessionToken] = {'gameId': INVALID_GAME_ID, 'players': [ { 'nickname': data["nickname"], 'heroId': data["heroId"] } ]}
                response = {
                    'error': '',
                    'data': INVALID_GAME_ID
                }
                return jsonify(response)

        return 'Unknown method in json'
    else:
        return 'Request method is not allowed'

if __name__ == '__main__':
    app.run(debug=True, port=service_port_global + service_port_local)