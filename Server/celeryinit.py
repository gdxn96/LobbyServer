from celery import Celery
from celery.bin import worker
from celery import current_app

def make_celery(app):
    celery = Celery(app.import_name, backend=app.config['CELERY_RESULT_BACKEND'],
                    broker=app.config['CELERY_BROKER_URL'])
    celery.conf.update(app.config)
    TaskBase = celery.Task
    class ContextTask(TaskBase):
        abstract = True
        def __call__(self, *args, **kwargs):
            with app.app_context():
                return TaskBase.__call__(self, *args, **kwargs)
    celery.Task = ContextTask
    celery.config_from_object('app_settings')
    return celery

def run_worker(app):
    application = current_app._get_current_object()

    w = worker.worker(app=application)

    options = {
        'broker': app.config['CELERY_BROKER_URL'],
        'loglevel': 'INFO',
        'traceback': True,
    }
    print "* \n" * 80
    w.run(**options)

def kill_worker(app):
    application = current_app._get_current_object()
    application.control.purge()