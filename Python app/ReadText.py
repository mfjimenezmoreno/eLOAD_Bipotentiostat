counts=dict()
print('Enter line of text')
line=input()
words=line.split()
print('Words:', words)
print('Counting...')
for word in words:
    counts[word]=counts.get(word, 0) + 1    #De esta manera podemos contar las palabras y asignarlas a un histograma dentro del diccionario
print('Counts', counts)
