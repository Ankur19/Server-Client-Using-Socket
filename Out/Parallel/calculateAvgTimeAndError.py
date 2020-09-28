import os

def calculateAverageErrors():
    files = os.listdir("./Errors")
    results = []
    for file in files:
        with open("./Errors/"+file, 'r') as f:
            cur = []
            for line in f:
                cur.append(int(line.strip('\n')))
            results.append(sum(cur)/len(cur))
    return results

def calculateAverageTimes():
    files = os.listdir("./Times")
    results = []
    for file in files:
        with open("./Times/"+file, 'r') as f:
            cur = []
            for line in f:
                cur.append(float(line.strip('\n')))
            results.append(sum(cur)/len(cur))
    return results

if __name__== "__main__":
    errors = calculateAverageErrors()
    times = calculateAverageTimes()

    print("Avg Errors: ")
    print(errors)
    print("Avg Times: ")
    print(times)

    print("Overall average error: " + str(sum(errors)/len(errors)))
    print("Overall average time: " + str(sum(times)/len(times)))

    with open("results.txt",'w') as f:
        f.write("Errors\n")
        f.write(str(errors))
        f.write('\n')
        f.write("Times\n")
        f.write(str(times))
        f.write('\n')
        f.write("Overall average error: " + str(sum(errors)/len(errors)) + '\n')
        f.write("Overall average time: " + str(sum(times)/len(times)) + '\n')
