def to_bb(n):
    r = []
    for i in range(8):
        s = ''
        for j in range(8):
            s += str(n%2)
            n /= 2
        r.append(s)
    r.reverse()
    return '\n'.join(r)


