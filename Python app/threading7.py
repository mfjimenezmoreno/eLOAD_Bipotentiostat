import threading
import time
import logging

"""It is also possible to pass a float representing the number of seconds to wait for the thread
to become inactive. Even if the thread didn't finish -> Join() returns anyway"""


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

d.join(0.1)   #Timeout of 100 ms
print('d.isAlive()', d.isAlive())
t.join()  # Waits for this one, but is already finished once called

"""In this case, it should print it is alive! We didn't wait until it was finished, our timeout is shorter"""
