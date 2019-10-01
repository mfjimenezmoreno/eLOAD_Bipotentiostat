def quadruple_repeat_decorator(a_function):
    def wrapper():
        l = []
        for i in range(4):
            l += [a_function]
        return(" ".join(l))

    return(wrapper)


@quadruple_repeat_decorator
def hey_ey():
    return("Hey-ey!")

print(hey_ey())