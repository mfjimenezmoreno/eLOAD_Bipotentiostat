from concurrent.futures import ThreadPoolExecutor
import threading
import random
import logging

logging.basicConfig(
    level=logging.DEBUG,
    format='(%(threadName)-10s) %(message)s',
)

def task():
    logging.debug('Executing task')
    result = 0
    i = 0
    for i in range(20):
        result = result + 1
    logging.debug("I: {}".format(result))
    logging.debug("Task executed {}".format(threading.current_thread()))

def main():
    executor = ThreadPoolExecutor(max_workers=3)
    executor.submit(task)
    executor.submit(task)
    
if __name__=='__main__':
    main()