########## Flask
FLASK_DEBUG=False
FLASK_PORT=5001

########## SQLAlchemy
SQLALCHEMY_DATABASE_URI = 'sqlite:///./db/demo.db'
SQLALCHEMY_TRACK_MODIFICATIONS = False

########## Celery Broker (Redis)
CELERY_BROKER_URL='redis://localhost:6379'
CELERY_RESULT_BACKEND='redis://localhost:6379'
CELERY_TASK_SERIALIZER='pickle'
CELERY_GRACEFUL_STOP = False

########## UDP Server
UDP_HOST = ''
UDP_PORT = 8888

########## Client Settings
CLIENT_PORT = 5226