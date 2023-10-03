for i in range(0, 61):
    print("""./gen_ft8 "CQ %03d" %s.wav""" % (i, i) + " %s" % str(50 * i))
    print("""sox -v 0.7 %s.wav %s.tmp.wav; mv %s.tmp.wav %s.wav""" % (i, i, i, i))
