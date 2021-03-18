import os,sys
import math
import multiprocessing
import regex,re

class file_detect:
    do_pinyin=False
    do_han = False
    def __init__(self, lines_:list, outfilenmae:str, isu:str):
        self.num_ker=1
        self.lines=lines_
        self.output_path=outfilenmae
        if isu == 'pinyin':
            self.do_pinyin=True
        elif isu == 'han':
            self.do_han = True
            
    def text_pro(self, text):
        #text = self.han.subf('')
        if self.do_han:
            match_obj = re.findall(r'[\u4e00-\u9fa5]', text)
        if self.do_pinyin:
            match_obj = re.findall(r'[A-Za-z]+\d', text)
        return match_obj

    def file_exi(self, index_list:list, process_index:int):
        f=open(self.output_path+"."+str(process_index), 'w', encoding='utf-8')
        for c in index_list:
            arr = self.lines[c].strip().split()
            line = self.text_pro("".join(arr[1:]))
            if len(self.lines[c].strip().split()) > 1:
                #print(arr[0])
                if self.do_han:
                    f.write(arr[0] + " " + "".join(line) + "\n")
                elif self.do_pinyin:
                    f.write(arr[0] + " " + " ".join(line) + "\n")
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
        s=file_detect(lines, sys.argv[2], sys.argv[3])
        s.multiProcess()
        s.over()

if __name__=='__main__':
    main()