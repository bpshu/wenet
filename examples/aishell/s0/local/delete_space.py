import os,sys
import math
import multiprocessing

class file_detect:
    def __init__(self, lines_:list, outfilenmae:str):
        self.num_ker=40
        self.lines=lines_
        self.output_path=outfilenmae

    def file_exi(self, index_list:list, process_index:int):
        f=open(self.output_path+"."+str(process_index), 'w', encoding='utf-8')
        for c in index_list:
            arr = self.lines[c].strip().split()
            line = arr[0] + " " + "".join(arr[1:])
            f.write(line + "\n")
        f.close()

    def chunks(self, arr:list, m:int):
        # m : num of process
        n = int(math.ceil(len(arr) / float(m)))
        return [arr[i:i + n] for i in range(0, len(arr), n)]

    def multiProcess(self):
        x = self.chunks(list(range(len(self.lines))), self.num_ker)
        process_list = []
        for i in range(self.num_ker):
            if i >= len(x):
                print("默认设置线程数目： ", str(self.num_ker), ", 但样本量不够均分，所以实际run线程数目  :", str(len(x)))
                continue
            p = multiprocessing.Process(target=self.file_exi, args=(x[i], i, ))
            p.start()
            process_list.append(p)

        for p in process_list:
            p.join()

    def over(self):
        cmd = "cat " + self.output_path + ".* | sort -u  > " + self.output_path
        os.system(cmd)
        cmd = "rm -rf  " + self.output_path + ".*"
        os.system(cmd)


def main():
    wav_scp=sys.argv[1]
    with open(wav_scp, 'r', encoding='utf-8') as f:
        lines=f.readlines()
        s=file_detect(lines, sys.argv[2])
        s.multiProcess()
        s.over()

if __name__=='__main__':
    main()