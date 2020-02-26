重现步骤：
1.）生成所包含所有图片地址的 *.txt，运行“ProduceCropData.cpp”，生成crop后的图片；
2.）生成包含“profile”下所有crop后图片的*.txt，使用“ProduceData_CasiaCeFA.cpp”，进行图像增广；
3.）生成包含“depth”下所有crop后图片的*.txt，使用“ProduceData_CasiaCeFA_depth.cpp”，进行图像增广；

4.）训练平台为caffe，train.prototxt/solver.prototxt/deploy.prototxt为训练和测试所需的  网络结构/配置文件/验证结构 。使用2.）和3.）步增广后的数据和原始数据分别训练“活体深度判别模型”、“活体IR判别模型”；

5.）使用训练好的模型，批量跑test数据，每个test id使用投票的方式确定该id的标签。 先使用“活体深度判别模型”，使用的阈值为0.5，如果 第x个test id下所有深度图片有一半以上被判断为攻击的概率大于阈值，则该id被判断为攻击；反之，使用“活体IR判别模型”，判断该id所有IR图片，使用的阈值为0.5，如果有一半以上被判断为攻击，则判别该id为攻击，反之判断为真人。

注：训练出来的模型还未更新至最新版，之后将选取效果最佳的上传
