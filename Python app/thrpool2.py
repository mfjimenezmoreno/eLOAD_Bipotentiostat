from concurrent.futures import ThreadPoolExecutor
import threading
import random
import logging

logging.basicConfig(
    level=logging.DEBUG,
    format='(%(threadName)-10s) %(message)s',
)

def task(n):
    logging.debug('Processing {}'.format(n))
    
def main():
    logging.debug("Starting ThreadpoolExecutor")
    with ThreadPoolExecutor(max_workers=3) as executor:
        future = executor.submit(task, (2))
        future = executor.submit(task, (3))
        future = executor.submit(task, (4))
    logging.debug('All tasks complete')

if __name__ == '__main__':
    main()