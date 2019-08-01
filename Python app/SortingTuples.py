c = {'a':10,'b':1,'c':22}

#A supercompressed way to sort a dictionary by highest values
#items returns keys and values as tuples, then the for iteration flips the tuples value/key, and finally sorted function takes place
print(sorted([ (v,k) for k,v in c.items() ]))
