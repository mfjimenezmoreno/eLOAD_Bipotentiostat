import random
import threading
import time
import logging

"""It is not necessary to have an explicit handle for daemon threads in order to ensure they finished before exiting
the main process. enumerate() is the magic function that returns a list of all active Thread instances. Warning: This list
includes the current thread, and since joining the current thread introduces a deadlock, must be skiped"""

def worker():
    """thread worker function"""
    pause = random.randint(1, 5) / 10
    logging.debug('sleeping %0.2f', pause)
    time.sleep(pause)
    logging.debug('ending')


logging.basicConfig(
    level=logging.DEBUG,
    format='(%(threadName)-10s) %(message)s',
)

for i in range(5):
    t = threading.Thread(target=worker, daemon=True)
    t.start()

main_thread = threading.main_thread()
for t in threading.enumerate():
    if t is main_thread:        #We add this to skip joining the main_thread, otherwise it will deadlock!
        continue
    logging.debug('joining %s', t.getName())
    t.join()

"""Look how enumerate allowed us to get the total number of pending daemon threads and subsequently ask for main to
wait for their completion

Also here we learned how to assign multiple unnamed daemons with an iterator"""