import threading
import time

#Using arguments to identify a thread is quite unnecesary and cumbersome.
#It is more simple to name the thread, also helps for server operations

def worker():
    print(threading.current_thread().getName(), 'Starting')
    time.sleep(0.2)
    print(threading.current_thread().getName(), 'Exiting')

def my_service():
    print(threading.current_thread().getName(), 'Starting')
    time.sleep(0.3)
    print(threading.current_thread().getName(), 'Exiting')

t = threading.Thread(name='my_service', target=my_service)
w = threading.Thread(name='worker', target=worker)
w2 = threading.Thread(target=worker)    #Use default name

w.start()
w2.start()
t.start()

# We see here that w has a service name meanwhile w2 doesn't, as we didn't name it.