import celeryinit
from server import app
import atexit

def kill_all_jobs(worker_app):
    print 'Shutting down worker'
    celeryinit.kill_worker(worker_app)

if __name__ == "__main__":
	kill_all_jobs(app)
	atexit.register(kill_all_jobs, app)
	celeryinit.run_worker(app)