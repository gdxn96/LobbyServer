import celeryinit
from server import app
import atexit

def goodbye(worker_app):
    print 'Shutting down worker'
    celeryinit.kill_worker(worker_app)

if __name__ == "__main__":
	atexit.register(goodbye, app)
	celeryinit.run_worker(app)
	