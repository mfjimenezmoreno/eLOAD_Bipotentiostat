"""In addition to synchronizing threads, it is important to share resources among them.
It is imoprtant to control the access to these shared resources to prevent corruption
or missed data.

Pythn in-built data structures (dictionaries, lists) are thread-safe as a side effect of
having atomic byte-codes for manipulating them. Other data structures, or even simpler ones
like integers or float, are not protected.

To guard against simultaneous access to an object, use a Lock object."""

import threading
import logging
import time
import random

class Counter:
    def __init__(self, start=0):
        self.lock = threading.Lock()
        self.value = start

    def increment(self):
        logging.debug('Waiting for lock')
        self.lock.acquire()                     #This should prevent anyother thread from changing self.value
        try:
            logging.debug('Acquiring lock')
            self.value = self.value + 1
        finally:
            self.lock.release()                 #Release the lock

def worker(c):
    for i in range(2):
        pause = random.random()
        logging.debug('Sleeping %0.02f', pause)
        time.sleep(pause)
        c.increment()
    logging.debug('Done')

logging.basicConfig(
    level=logging.DEBUG,
    format = '(%(threadName)-10s) %(message)s',
)

counter = Counter()
for i in range(2):
    t = threading.Thread(target=worker, args=(counter,))
    t.start()

logging.debug('Waiting for worker threads')
main_thread = threading.main_thread()
for t in threading.enumerate():
    if t is not main_thread:
        t.join()
logging.debug('Counter: %d', counter.value)
