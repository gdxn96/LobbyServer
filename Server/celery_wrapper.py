from server import celery

@celery.task()
def celery_task(task, *args, **kwargs):
	task(*args, **kwargs)

@celery.task()
def daemon_task(task, *args, **kwargs):
	print "Start daemon {}".format(task.__name__)
	while True:
		print "Refresh daemon {}".format(task.__name__)
		try:
			task(*args, **kwargs)
		except:
			pass


def start_task(task, *args, **kwargs):
	celery_task.delay(task, *args, **kwargs)

def start_daemon_task(task, *args, **kwargs):
	daemon_task.delay(task, *args, **kwargs)