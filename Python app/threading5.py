import threading
import time
import logging

"""So far, our programs have waited for all threads to finish...
sometimes it is required that a program spawns threads without interrupting the main one from exiting.
This is known as a daemon thread. Useful for services where there may not be an easy way to interrupt
the thread, or where letting the thread die in the middle of its work does not corrupt the data
ex. a thread generates heartbeats for a monitor tool service """


def daemon():
    logging.debug('Starting')
    time.sleep(1)
    logging.debug('Exiting')


def non_daemon():
    logging.debug('Starting')
    logging.debug('Exiting')

"""We need to format the debugger with this function"""
logging.basicConfig(
    level=logging.DEBUG,
    format='[%(levelname)s] (%(threadName)-10s) %(message)s'
)

d = threading.Thread(name='daemon', target=daemon, daemon=True)
t = threading.Thread(name='non-daemon', target=non_daemon, daemon=False)

d.start()
t.start()

"""As you can see, the daemon exiting process is not completed, as the main thread and non-daemon finished
first, and didn't wait for it to finish! It got stuck there in the time critical sleep call"""

