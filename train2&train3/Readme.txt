
train_2&3重现步骤：
1.）使用“ProduceData_CasiaCeFA_DepthAndIR.cpp”生成IR和Depth数据，里面包含了数据增广内容

2.）模型训练使用的训练平台为caffe，train.prototxt/solver.prototxt/deploy.prototxt为训练和测试所需的  网络结构/配置文件/验证结构 。使用2.）和3.）步增广后的数据和原始数据分别训练“活体深度判别模型”、“活体IR判别模型”；

5.）模型训练好后，参考“TestModel_CASIA_CeFA_All.cpp”得到结果

（注：train_2 和 train_3 的数据处理方式不同，在调用“ProduceData_CasiaCeFA_DepthAndIR.cpp”时需要按照训练集的不同，打开和关闭不同的宏定义）
