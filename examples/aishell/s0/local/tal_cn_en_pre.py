import os,sys
import math
import multiprocessing
import regex,re

class file_detect:
    han = regex.compile(r'\p{Han}')
    en_word=regex.compile(r'[A-Za-z]')
    special_sym=regex.compile(r'\'')
    punct = regex.compile(r'\p{Punct}')
    special_sym1=regex.compile(r'\’|\‘')
    #han_space = regex.compile(r'(\p{Han})\s+(\p{Han})|(\p{Han})\s+([A-Za-z])|([A-Za-z])\s+(\p{Han})')
    han_space1 = regex.compile(r'(\p{Han})\s+(\p{Han})')
    han_space2 = regex.compile(r'(\p{Han})\s+([A-Za-z])')
    han_space3 = regex.compile(r'([A-Za-z])\s+(\p{Han})')
    #han_space4 = regex.compile(r'((\p{Han})\s+(\')')
    #space_patten = re.compile(r'([\u4e00-\u9fa5])\s+([\u4e00-\u9fa5])')

    def __init__(self, lines_:list, outfilenmae:str):
        self.num_ker=40
        self.lines=lines_
        self.output_path=outfilenmae

    def text_post(self, text):
        #text = "哇，的 'word,don't don’‘t english' english 平第"
        text = self.special_sym1.subf('\'', text)
        text = text.replace('\'\'', '\'')
        text = self.special_sym.subf('AAAAAA', text)
        text = self.punct.subf(' ', text)
        text = text.replace('AAAAAA', '\'')
        text = self.han_space1.subf(r'{1}{2}', text)
        text = self.han_space2.subf(r'{1}{2}', text)
        text = self.han_space3.subf(r'{1}{2}', text)
        #text = self.han_space4.subf(r'{1}{2}', text)
        #text = self.space_patten.sub(r'\1\2', text)
        return text

    def file_exi(self, index_list:list, process_index:int):
        f=open(self.output_path+"."+str(process_index), 'w', encoding='utf-8')
        for c in index_list:
            line = " ".join(self.lines[c].strip().split()[1:])
            line = self.text_post(line)
            #print(line)
            #exit(0)
            if len(self.lines[c].strip().split()) > 1:
                f.write(self.lines[c].strip().split()[0]+" "+line + "\n")
            # exit(0)
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