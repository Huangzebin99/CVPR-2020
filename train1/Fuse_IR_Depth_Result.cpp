#include <iostream>
#include <string>
#include <fstream>

using namespace std;

void main()
{
	ifstream FtpIR("4@1_sum_ir.txt");
	ifstream FtpDepth("4@1_sum_depth.txt");
	ofstream FtpFuse("4@1_sum_fuse.txt");

	string LineIR, LineDepth;
	char LabelIR, LabelDepth;

	while (getline(FtpIR,LineIR))
	{
		getline(FtpDepth, LineDepth);

		LabelDepth = LineDepth[LineDepth.length() - 1];

		if ('0' == LabelDepth)		// 
			FtpFuse << LineDepth << endl;
		else
			FtpFuse << LineIR << endl;
	}
	FtpIR.close();
	FtpDepth.close();
	FtpFuse.close();
}