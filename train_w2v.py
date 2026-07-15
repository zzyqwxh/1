#导入使用的包
from gensim.models.word2vec import Word2Vec
import jieba
#编写word2vec增量训练类
class TrainWord2Vec:
    # 初始化方法
    def __init__(self, stopwords_file,
                 num_features=50,
                 min_word_count=1,
                 context=5,
                 incremental=False,
                 old_path=None):
        """
        定义变量
        :param stopword: 停用词表
        :param num_features:  返回的向量长度
        :param min_word_count:  最低词频
        :param context: 滑动窗口大小
        :param incremental: 是否进行增量训练
        :param old_path: 若进行增量训练，原始模型路径
        """
        # 加载停用词
        self.stopwords=self._get_stopwords(stopwords_file)
        self.num_features = num_features
        self.min_word_count = min_word_count
        self.context = context
        self.incremental = incremental
        self.old_path = old_path
    def _get_stopwords(self,file_path):
        """
        该方法用来从停用词文件中加载停用词
        :param file_path: 停用词路径
        :return: 停用词集合
        """
        #创建集合
        stopwords = set()
        #打开文件并遍历每一行
        with open(file_path, 'r') as infile:
            for line in infile:
                line = line.rstrip('\n')#去掉右边的换行符
                if line:
                    stopwords.add(line.lower())#转换成小写后添加到集合中
        return stopwords
    def get_text(self,content_file):
        """
        该方法从训练数据中读取文本数据并进行分词，去除停用词，最终返回分词之后的文章列表
        :param file_path: 训练文件路径  每行结构： 文本 \t 标签
        :param stopwords_file: 停用词文件路径
        :return: 由每行文本的词构成的列表的列表
        """
        corpus = []
        # 打开文本 遍历每一行
        for line in open(content_file, 'r', encoding='utf-8'):
            curr_words=[]
            # 遍历每一行中的每个词
            for word in jieba.cut(line.strip().split('\t')[0]):
                # 去除停用词
                if word not in self.stopwords:
                    curr_words.append(word)
            corpus.append(curr_words)
        print(corpus)
        return corpus
    def get_model(self, text):
        """
        从头训练word2vec模型
        :param text: 经过清洗之后的语料数据
        :return: word2vec模型
        """
        model = Word2Vec(corpus_iterable=text,
                     size=self.num_features,
                     min_count=self.min_word_count,
                     window=self.context)
        return model
    def update_model(self, text):
        """
        增量训练word2vec模型
        :param text: 经过清洗之后的新的语料数据
        :return: word2vec模型
        """
        model = Word2Vec.load(self.old_path)  # 加载旧模型
        model.build_vocab(text, update=True)  # 更新词汇表
        model.train(corpus_iterable=text, total_examples=model.corpus_count,epochs=model.epochs)
        return model
    def run(self,content_file,save_path):
        """
        负责完成整个训练和保存的流程
        :param content_file: 训练预料文件路径
        :param save_path: 词向量模型保存路径
        :return: None
        """
        #提取文本分词结果
        text = self.get_text(content_file)
        if self.incremental:#判断是否增量训练
            model = self.update_model(text)#增量训练
        else:
            model = self.get_model(text)#重新训练
        # 保存模型
        model.save(save_path+"/word2vec_new.model")#保存模型
if __name__ == '__main__':
    stopword='95598_1/stopwords.txt'# 停用词文件路径
    content_file = '95598_1/train.txt'# 训练语料文件路径
    save_path = 'vectors'# 增量训练后的word2vec模型文件路径
    old_path = '95598_1/souhu.w2v'# 预训练的word2vec模型词向量文件路径
    #实例化对象
    trainmodel = TrainWord2Vec(incremental=True,
                               stopwords_file=stopword,
                               old_path=old_path)
    #训练并保存
    trainmodel.run(content_file,save_path)
