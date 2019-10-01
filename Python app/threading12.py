"""The idea of multiple threads is to have some independency, but sometimes
we will require synchronization. Event objects are a simple way to communicate
between threads safely.

An event manages an internal flag that callers can control with set and clear methods.
Other threads can use wait to pause until flag is set."""

import logging
import threading
import time

def wait_for_event(e):
    """Wait for event to be set before doing anything"""
    logging.debug('Wait_for_event_timeout starting')
    event_is_set = e.wait()                             #It will get stuck here!
    logging.debug('Event set: %s', event_is_set)

def Wait_for_event_timeout(e, t):
    """Wait t seconds and then timeout"""
    while not e.is_set():                               #Repeat until
        logging.debug('Wait_for_event_timeout starting')
        event_is_set = e.wait(t)                        #Here we added a timeout, it can escape in this example after 2 seconds!
        logging.debug('event set: %s', event_is_set)    #For us to know if the event is set or not!
        if event_is_set:
            logging.debug('Processing event')
        else:
            logging.debug('doing other work')

logging.basicConfig(
    level=logging.DEBUG,
    format='%(threadName)-10s %(message)s'
)

e = threading.Event()
t1 = threading.Thread(
    name='block',
    target=wait_for_event,
    args=(e,),
    )
t1.start()
t2 = threading.Thread(
    name='non block',
    target=Wait_for_event_timeout,
    args=(e, 2),
)
t2.start()

logging.debug('Waiting before calling calling Event.set()')
time.sleep(5)
e.set()
logging.debug('Even is set')