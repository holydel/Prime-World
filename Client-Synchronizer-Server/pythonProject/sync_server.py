from waitress import serve
from client_sync import app

if __name__ == '__main__':
    serve(app, host='127.0.0.1', port='36530', threads=24)