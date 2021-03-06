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
    logging.debug("Starting ThreadpoolExecutot")
    with ThreadPoolExecutor(max_workers=3) as executor:
        future = executor.submit(task, (2))
        future = executor.submit(task, (2))
        future = executor.submit(task, (2))