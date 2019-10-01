import threading
import time
import logging

"""So... how do we wait for the daemon thread to finish its work?
We use join() method as follows"""

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

d.join()                #This locks the thread (main in this case) from continuing until thread id finished indifinitely
print('Liiiisto')
t.join()                #Waits for this one, but is already finished once called
print('Este también')
