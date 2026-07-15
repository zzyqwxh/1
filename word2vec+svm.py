from gensim.models import Word2Vec
import numpy as np
import jieba#jieba分词
from sklearn.svm import LinearSVC
from sklearn.metrics import classification_report#分类报告函数
class LrW2vModel(object):
    def __init__(self,w2v,labels):
        """
        初始化模型
        :param w2v: 词向量模型路径
        :param labels: 标签文件路径
        """
        #加载词向量文件
        self.w2v=self._load_w2v(w2v)
        #加载标签
        self.class_list = [x.strip() for x in open(labels, encoding='utf-8').readlines()]
        #初始化逻辑回归模型
        self.model = LinearSVC(C=10, multi_class='ovr', max_iter=2000, random_state=42)
    #加载词向量模型
    def _load_w2v(self,path):
        w2v = Word2Vec.load(path)
        return w2v
    def _get_text_label(self,file):
        """
        获取文本词列表和标签
        :param file: 训练文本文件路径
        :return: 文本的词列表，文本标签
        """
        articles=[]
        labels=[]
        # 打开文本 遍历每一行
        for line in open(file, 'r', encoding='utf-8'):
            text,label=line.strip().split('\t') #提取文本内容和标签
            articles.append(jieba.lcut(text)) #文本分词
            labels.append(label)
        lens=np.array([len(x) for x in articles])
        print("Longest:{},Shortest:{},Average:{}".format(lens.max(),lens.min(),lens.mean()))
        return articles,np.array(labels).astype(np.int)
    def _compute_doc_vec_single(self,article):
        vec = np.zeros((self.w2v.layer1_size,),     dtype=np.float32)
        n = 0
    # 把 self.w2v 改成 self.w2v.wv
        for word in article:
            if word in self.w2v.wv:
                vec += self.w2v.wv[word]
                n += 1
        if n == 0:
            return vec
        return vec / n
    #计算文本的词向量表示
    def _compute_doc_vec(self,articles):
        #遍历每个文本，计算文本向量表示
        return np.row_stack([self._compute_doc_vec_single(x) for x in articles])
    def train(self,data):
        #1.处理数据
        articles,labels=self._get_text_label(data)
        #2.计算句向量
        x=self._compute_doc_vec(articles)
        #3.模型训练
        self.model.fit(x,labels)
    def evaluate(self,data):
        #1.处理数据
        articles, labels = self._get_text_label(data)
        # 2.计算句向量
        x = self._compute_doc_vec(articles)
        #3.模型预测
        y_pred=self.model.predict(x)
        #4.模型评估 分类报告
        return classification_report(y_pred=y_pred,
                                     y_true=labels ,
                                     labels=[0,1,2,3,4,5,6,7],
                                     target_names=self.class_list)
if __name__ == '__main__':
    import time
    train_start = time.time()
    train_data='95598_1/train.txt'
    dev_data = '95598_1/test.txt'
    w2v='vectors/word2vec_new.model'
    labels='95598_1/class.txt'
    model=LrW2vModel(w2v,labels)
    model.train(train_data)
    train_end = time.time()
    print(f"SVM训练耗时：{train_end - train_start:.4f} s")
    print("===== Word2Vec+SVM 分类报告 =====")
    print(model.evaluate(dev_data))
