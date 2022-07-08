import os

# TODO: change command

if "MODE" in os.environ: MODE = os.environ["MODE"]
else: MODE = "PROD"

errors = []

def run_test(path, command, args, success=True):

    res = [0, 0]

    for file in os.listdir(path):

        if ((os.system(command + ' ' + args + ' ' + os.path.join(path, file)) == 0) == success):
            print(" \033[32m[ OK ]\33[0m   " + file)
            res[0] += 1 
        else:
            errors.append(file)
            print(" \033[31m[ KO ]\33[0m   " + file)
        res[1] += 1
    #print(res)
    return res

if __name__ == '__main__':

    print("------------------GOOD------------------------\n")

    sum_ok = run_test("./tests"\
            "/test_files_zone/ok", "./build/" + MODE + "/dnsd", "-c -f")

    print("\n------------------BAD------------------------\n")

    sum_ko = run_test("./tests"\
            "/test_files_zone/ko", "./build/" + MODE + "/dnsd", "-c -f", False)

    print("\n------------------SUMMARY------------------------\n")
    summary = [sum(i) for i in zip(sum_ok,sum_ko)]
    print("\nSUCCESS: " + str(summary[0]) + '/' + str(summary[1]))
    print("\nFAILURE: " + str(summary[1] - summary[0]) + '/' + str(summary[1]))
    for error in errors:
        print("\033[31m[" + error + ']\33[0m\n')
