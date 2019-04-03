// Project3.cpp : Defines the entry point for the console application.

#include <iostream>
#include <fstream>
#include <string>
#include <stdlib.h>
#include <math.h>

/*
	ACCURACY thể hiện độ chính sác của phân nhánh.
	Một node có số Rating chiếm hơn ACCURACY*100% thì được coi là node lá và có nhãn là Rating đó
*/
#define ACCURACY 0.7
#define PERCENT_VALIDATION_DATA 0.2
#define k 5

using namespace std;
/*
Customer lưu trữ các giá trị của một bản ghi.
Gồm các thuộc tính: ID, WC_TA, RE_TA, EBIT_TA, MVE_BVTD, S_TA, Industry, Rating.
Trong đó:
attribute[0]: WC_TA
attribute[1]: RE_TA
attribute[2]: EBIT_TA
attribute[3]: MVE_BVTD
attribute[4]: S_TA
Rating = 0 (AAA)
Rating = 1 (AA)
Rating = 2 (A)
Rating = 3 (BBB)
Rating = 4 (BB)
Rating = 5 (B)
Rating = 6 (CCC)
*/
struct Customer {
	short int Industry, Rating;
	int ID;
	float attribute[5];
};
/*
Các mảng trainData và studyData lưu dữ liệu của tệp traning và tệp test
Biến trainDataSize lưu số phần tử của mảng trainData
Biến studyDataSize lưu số phần tử của mảng studyData
*/
Customer *data = new Customer[5000];
Customer *trainData;					// (1 - PERCENT_TEST_DATA) * 100% data
Customer *studyData = new Customer[1500];// Study Data, default Rating = -1
Customer *validationData;						// PERCENT_TEST_DATA * 100% data
Customer *checkValidationData;				// 
int validationDataSize = 0;
int studyDataSize = 0;					// Size of Test Data
int trainDataSize;						// Size of Train Data
int dataSize;

/*
Node lưu các giá trị phục vụ cho việc tạo cây.
selectedAttribute:	Thuộc tính phân chia (0 -> 4). Mặc định là -1.
tempTrainData:		Mảng các dữ liệu traning bị chia thành cho từng node .
size:				Số phần tử của mảng tempTrainData.
tempTrainMore, tempTrainLess: Mảng các dữ liệu traning của node con phải và trái.
sizeMore, sizeLess:	Số phần tử của mảng tempTrainMore, tempTrainLess.
threshold:			Ngưỡng phân chia của selectedAttribute
isLeaf:				Node có phải là lá hay không. Mặc định là false.
label:				Nhãn của node. Là Rating chiếm đa số. Mặc định là -1.
leftChild, rightChild:	Con trỏ trỏ đến node con trái, phải của node hiện tại.
*/
struct Node {
	int selectedAttribute = -1;
	Customer *tempTrainData;
	Customer *tempTrainMore, *tempTrainLess;
	int sizeMore = 0, sizeLess = 0;
	int size;
	float threshold;
	bool isLeaf = false;
	int label = -1;
	Node *leftChild;
	Node *rightChild;
};

/*
Cấu trúc InfoGain:
Mục đích phục vụ cho lưu trữ dễ dàng, hiệu quả hơn.
*/
struct InfoGain {
	float gain = 0;
	int sizeLeft = 0, sizeRight = 0;
};

/*
Hàm checkLeaf kiểm tra node hiện tại có phải là node lá hay không.
Nếu có trả về nhãn của node đó (nhãn từ 0 -> 6)
Nếu không trả về -1.
*/
int checkLeaf(Node* currentNode) {
	int sizeThis[7], flag = -1;
	for (int i = 0; i < 7; i++)
		sizeThis[i] = 0;
	for (int i = 0; i < currentNode->size; i++) {
		sizeThis[currentNode->tempTrainData[i].Rating]++;
	}
	// for (int i = 0; i < 7; i++)
	// 	cout << sizeThis[i] << " ~~~ ";
	// cout << endl;
	for (int i = 0; i < 7; i++) {
		if (sizeThis[i] > ACCURACY * currentNode->size)
			flag = i;
	}
	return flag;
}

/*
Các hàm show phục vụ cho debug.
Hiển thị bảng của node hiện tại.
*/
void showCustomerLeft(Node *currentNode) {
	cout << "Size: " << currentNode->sizeLess << endl;
	for (int i = 0; i < currentNode->sizeLess; i++) {
		cout << currentNode->tempTrainLess[i].ID << "\t";
		for (int j = 0; j < 5; j++) {
			cout << currentNode->tempTrainLess[i].attribute[j] << "\t";
		}
		cout << currentNode->tempTrainLess[i].Rating << "\n";
	}
}
void showCustomerRight(Node *currentNode) {
	cout << "Size: " << currentNode->sizeMore << endl;
	for (int i = 0; i < currentNode->sizeMore; i++) {
		cout << currentNode->tempTrainMore[i].ID << "\t";
		for (int j = 0; j < 5; j++) {
			cout << currentNode->tempTrainMore[i].attribute[j] << "\t";
		}
		cout << currentNode->tempTrainMore[i].Rating << "\n";
	}
}
void showHistorical(Node *currentNode) {
	cout << "Size: " << currentNode->size << endl;
	for (int i = 0; i < currentNode->size; i++) {
		cout << i + 2 << "   " << currentNode->tempTrainData[i].ID << "\t";
		for (int j = 0; j < 5; j++) {
			cout << currentNode->tempTrainData[i].attribute[j] << "\t";
		}
		cout << currentNode->tempTrainData[i].Rating << "\n";
	}
}

/*
Hàm makeNode() tạo một node mới và cấp phát bộ nhớ cho nó.
*/
Node *makeNode() {
	Node *P = new Node();
	P->leftChild = NULL;
	P->rightChild = NULL;
	return P;
}

/*
Hàm readTraningFile đọc file training.
Các giá trị được lưu vào mảng Historical
*/
void readTraningFile(char* str, Customer *varCustomer, int &varSize) {
	fstream f;
	f.open(str, ios::in);
	string data;
	string line;
	int i = 0;
	getline(f, line);
	string temp = "";
	int flag = 0;
	while (!f.eof())
	{
		flag = 0;
		getline(f, line);
		temp = "";
		if (line == "\0") break;
		for (int j = 0; j < line.length(); j++) {
			if (line[j] != ',' && line[j] != '\0')
				temp += line[j];
			if (line[j] == ',') {
				flag++;
				if (flag == 1)
					varCustomer[i].ID = stoi(temp);
				if (flag == 2)
					varCustomer[i].attribute[0] = stof(temp);
				if (flag == 3)
					varCustomer[i].attribute[1] = stof(temp);
				if (flag == 4)
					varCustomer[i].attribute[2] = stof(temp);
				if (flag == 5)
					varCustomer[i].attribute[3] = stof(temp);
				if (flag == 6)
					varCustomer[i].attribute[4] = stof(temp);
				if (flag == 7)
					varCustomer[i].Industry = stoi(temp);
				temp = "";
			}
			if (j == line.length() - 1) {
				if (temp == "AAA")
					varCustomer[i].Rating = 0;
				if (temp == "AA")
					varCustomer[i].Rating = 1;
				if (temp == "A")
					varCustomer[i].Rating = 2;
				if (temp == "BBB")
					varCustomer[i].Rating = 3;
				if (temp == "BB")
					varCustomer[i].Rating = 4;
				if (temp == "B")
					varCustomer[i].Rating = 5;
				if (temp == "CCC")
					varCustomer[i].Rating = 6;
				if (temp == "")
					varCustomer[i].Rating = -1;
			}
		}
		i++;
	}
	varSize = i;
	f.close();
	cout << "Read " << str << " completed\n";
}

/*
Hàm readTestFile đọc file test.
Các giá trị được lưu vào mảng studyData.
*/
void readStudyFile(char* str, Customer *varCustomer, int &varSize) {
	fstream f;
	f.open(str, ios::in);
	string data;
	string line;
	int i = 0;
	getline(f, line);
	string temp = "";
	int flag = 0;
	while (!f.eof())
	{
		flag = 0;
		getline(f, line);
		temp = "";
		if (line == "\0") break;
		for (int j = 0; j < line.length(); j++) {
			if (line[j] != ',' && line[j] != '\0')
				temp += line[j];
			if (line[j] == ',') {
				flag++;
				if (flag == 1)
					varCustomer[i].ID = stoi(temp);
				if (flag == 2)
					varCustomer[i].attribute[0] = stof(temp);
				if (flag == 3)
					varCustomer[i].attribute[1] = stof(temp);
				if (flag == 4)
					varCustomer[i].attribute[2] = stof(temp);
				if (flag == 5)
					varCustomer[i].attribute[3] = stof(temp);
				if (flag == 6)
					varCustomer[i].attribute[4] = stof(temp);
				temp = "";
			}
			if (j == line.length() - 1) {
				varCustomer[i].Industry = stoi(temp);
			}
		}
		i++;
	}
	varSize = i;
	f.close();
	cout << "Read " << str << " completed\n";
}

/*
Hàm entropyT tính entropy một tham số.
Đầu vào là mảng lưu số lượng các Rating.
*/
float entropyT(int sizeOfRating[]) {
	int total = 0;
	for (int i = 0; i < 7; i++)
		total += sizeOfRating[i];
	float ent = 0;
	for (int i = 0; i < 7; i++) {
		if (sizeOfRating[i] != 0)
			ent -= (sizeOfRating[i] * 1.0 / total) * log2(sizeOfRating[i] * 1.0 / total);
	}
	return ent;
}
/*
	Hàm entropyTX tính entropy hai tham số.
	Đầu vào là mảng lưu số lượng các Rating
		nhỏ hơn và lớn hơn (sau khi chia bảng ra làm hai)
*/
float entropyTX(int sizeLessThan[], int sizeMoreThan[]) {
	int sum = 0, sumLess = 0, sumMore = 0;
	for (int i = 0; i < 7; i++) {
		sumLess += sizeLessThan[i];
		sumMore += sizeMoreThan[i];
	}
	sum = sumLess + sumMore;
	float x = sumLess*1.0 / sum * entropyT(sizeLessThan);
	float y = sumMore*1.0 / sum * entropyT(sizeMoreThan);
	return x + y;

}

/*
	Hàm getGain tính gain của node hiện tại.
*/
InfoGain getGain(Node* currentNode, int selectedAttribute, float threshold) {
	InfoGain p;
	p.gain = 0;
	int sizeMore[7], sizeLess[7], sizeThis[7];
	for (int i = 0; i < 7; i++) {
		sizeMore[i] = 0;
		sizeLess[i] = 0;
		sizeThis[i] = 0;
	}
	//		COUNT RATING
	for (int i = 0; i < currentNode->size; i++) {
		sizeThis[currentNode->tempTrainData[i].Rating]++;
		if (currentNode->tempTrainData[i].attribute[selectedAttribute] > threshold) {
			sizeMore[currentNode->tempTrainData[i].Rating]++;
			p.sizeRight++;
		}
		else {
			sizeLess[currentNode->tempTrainData[i].Rating]++;
			p.sizeLeft++;
		}
	}
	p.gain = entropyT(sizeThis) - entropyTX(sizeLess, sizeMore);
	return p;
}

/*
	Hàm setThresholds: tạo giá trị ngưỡng và thuộc tính phân chia cho node hiện tại.
	Các bước:
		Kiểm tra xem có phải node lá hay không.
			Nếu có thì set giá trị cho thuộc tính label
		Thử ngưỡng là từng giá trị trong bảng tempTrainData,
			giá trị nào đem lại gain cao nhất thì chọn được thuộc tính phân chia
			và ngưỡng của node hiện tại
*/
void setThresholds(Node *currentNode) {
	int flag = checkLeaf(currentNode);
	if (flag != -1) {
		currentNode->isLeaf = true;
		currentNode->label = flag;
	}
	else {
		/* Find threshold and selectedAttribute*/
		InfoGain globalInfo, universeInfo, localInfo;
		float localThreshold = 0, globalThreshold = 0;
		int selectedAttribute = -1;

		for (int i = 0; i < 5; i++) {
			globalInfo.gain = 0;
			for (int j = 0; j < currentNode->size; j++) {
				localInfo = getGain(currentNode, i, currentNode->tempTrainData[j].attribute[i]);
				if (localInfo.gain > globalInfo.gain) {
					globalInfo = localInfo;
					localThreshold = currentNode->tempTrainData[j].attribute[i];
				}
			}
			// cout << globalInfo.gain << " ";
			if (globalInfo.gain > universeInfo.gain) {
				selectedAttribute = i;
				universeInfo = globalInfo;
				globalThreshold = localThreshold;
			}
		}
		// cout << endl;
		// Set sizeMore, tempTrainLess
		Customer *tempTrainMore = new Customer[universeInfo.sizeRight];
		Customer *tempTrainLess = new Customer[universeInfo.sizeLeft];
		int countLeft = 0, countRight = 0, sizeThis[7];
		for (int i = 0; i < 7; i++)
			sizeThis[i] = 0;
		for (int i = 0; i < currentNode->size; i++) {
			if (currentNode->tempTrainData[i].attribute[selectedAttribute] > globalThreshold) {
				for (int j = 0; j<5; j++){
					tempTrainMore[countRight].attribute[j] = currentNode->tempTrainData[i].attribute[j];
				}
				tempTrainMore[countRight].ID = currentNode->tempTrainData[i].ID;
				tempTrainMore[countRight].Rating = currentNode->tempTrainData[i].Rating;
				tempTrainMore[countRight].Industry = currentNode->tempTrainData[i].Industry;
				countRight++;
			}else {
				for (int j = 0; j<5; j++){
					tempTrainLess[countLeft].attribute[j] = currentNode->tempTrainData[i].attribute[j];
				}
				tempTrainLess[countLeft].ID = currentNode->tempTrainData[i].ID;
				tempTrainLess[countLeft].Rating = currentNode->tempTrainData[i].Rating;
				tempTrainLess[countLeft].Industry = currentNode->tempTrainData[i].Industry;
				countLeft++;
			}
		}
		currentNode->tempTrainLess = tempTrainLess;
		currentNode->tempTrainMore = tempTrainMore;
		currentNode->sizeLess = countLeft;
		currentNode->sizeMore = countRight;
		// cout << "Selected Attribute = " << selectedAttribute << endl;
		currentNode->selectedAttribute = selectedAttribute;
		currentNode->threshold = globalThreshold;
	}
	// std::cout << "Threshold = " << currentNode->threshold <<'\n';
	// cout << "FLAG = " << currentNode->label << endl;
	// cout << "------------------------------------------------------" << endl;
}
/*
Hàm makeLeftChild, makeRightChild: tạo node con trái và node con phải
cho node hiện tại
*/
Node *makeLeftChild(Node* currentNode) {
	Node* P = makeNode();
	P->tempTrainData = currentNode->tempTrainLess;
	P->size = currentNode->sizeLess;
	setThresholds(P);
	return P;
}
Node *makeRightChild(Node* currentNode) {
	Node* P = makeNode();
	P->tempTrainData = currentNode->tempTrainMore;
	P->size = currentNode->sizeMore;
	setThresholds(P);
	return P;
}

/*
Hàm generateTree: Tạo cây bằng đệ quy
*/
void generateTree(Node *root) {
	if (root != NULL && root->label == -1) {
		root->leftChild = makeLeftChild(root);
		if (root->leftChild->label == -1)
			generateTree(root->leftChild);
		root->rightChild = makeRightChild(root);
		if (root->rightChild->label == -1)
			generateTree(root->rightChild);
	}
}
/*
Hàm displayCustomer: Hiển thị initCustomer
(Chỉ dùng để debug)
*/
void displayCustomer(Customer initCustomer[], int initSize) {
	cout << "Size: " << initSize << endl;
	char* str;
	for (int i = 0; i < initSize; i++) {
		cout << i + 2 << "   " << initCustomer[i].ID << "\t";
		for (int j = 0; j < 5; j++) {
			cout << initCustomer[i].attribute[j] << "\t";
		}
		cout << initCustomer[i].Industry << "\t";
		switch (initCustomer[i].Rating){
			case 0: str = (char*)"AAA"; break;
			case 1: str = (char*)"AA"; break;
			case 2: str = (char*)"A"; break;
			case 3: str = (char*)"BBB"; break;
			case 4: str = (char*)"BB"; break;
			case 5: str = (char*)"B"; break;
			case 6: str = (char*)"CCC"; break;
			default:
				str = (char*)"unknow";
				break;
		}
		cout << str << endl;
	}
}

/*
	Train
*/
void train(Node* rootNode, Customer* data, int dataSize) {
	Node* cur = rootNode;
	for (int i = 0; i < dataSize; i++) {
		cur = rootNode;
		while (cur->label == -1) {
			if (data[i].attribute[cur->selectedAttribute] > cur->threshold)
				cur = cur->rightChild;
			else
				cur = cur->leftChild;
		}
		data[i].Rating = cur->label;
	}
}

void generateFile(char* outputDir, Customer* data, int dataSize){
	ofstream myfile;
	myfile.open(outputDir);
	myfile << "ID,WC_TA,RE_TA,EBIT_TA,MVE_BVTD,S_TA,Industry,Rating\n";
	char* str;
	for (int i = 0; i < dataSize; i++) {
		switch (studyData[i].Rating)
		{
		case 0: str = (char*)"AAA"; break;
		case 1: str = (char*)"AA"; break;
		case 2: str = (char*)"A"; break;
		case 3: str = (char*)"BBB"; break;
		case 4: str = (char*)"BB"; break;
		case 5: str = (char*)"B"; break;
		case 6: str = (char*)"CCC"; break;
		default:
			str = (char*)"unknow";
			break;
		}
		myfile << data[i].ID << "," << data[i].attribute[0] << "," << data[i].attribute[1] << "," <<
			data[i].attribute[2] << "," << data[i].attribute[3] << "," << data[i].attribute[4] << "," <<
			data[i].Industry << "," << str << "\n";
	}
	myfile.close();
}

/*
	Kiem tra chat luong hoc
*/
float evaluateTrainQuality(Customer *checkValidationData, Customer *validationData, int size) {
	int** statistics = new int*[7];
	for(int i = 0; i < 7; ++i) {
    statistics[i] = new int[7];
	}
	for (int i=0; i<7; i++)
		for (int j=0; j<7; j++)
			statistics[i][j] = 0;

	int correct=0;
	int wrong = 0;
	int near = 0;
	for (int i=0; i<size; i++){
		if (checkValidationData[i].Rating == validationData[i].Rating){
			correct++;
		}else if(abs(checkValidationData[i].Rating - validationData[i].Rating) == 1){
			near++;
		}
		statistics[checkValidationData[i].Rating][validationData[i].Rating]++;
	}
	for (int i=0; i<7; i++){
		for (int j=0; j<7; j++){
			printf("%4d", statistics[i][j]);
		}
		cout<<endl;
	}
	wrong = size - correct;
	float raito = correct*1.0/size;
	cout<<"Size: "<<size<<"; Correct: "<<correct<<"; Wrong: "<<wrong<<"; Near: "<< near <<"; Raito: "<<raito<<endl;
	return raito;
}

int main(int argc, char* argv[]){	
	if (argc == 4 || argc == 2){
		Node* root;
		readTraningFile(argv[1], data, dataSize);
		int count = 0;
		float averageRaito = 0;
		int div = dataSize/k;
		int mod = dataSize%k;
		int countTrain = 0, countValidation = 0;
		trainData = new Customer[div*(k-1) + mod];
		validationData = new Customer[div + mod];
		for (int i=0; i<k; i++){
			cout<<"--------------k = "<<i+1<<"--------------"<<endl;
			countTrain = 0, countValidation = 0;
			if (i==k-1){
				trainDataSize = div*(k-1);
				validationDataSize = div + mod;
				trainData = new Customer[trainDataSize];
				validationData = new Customer[validationDataSize];
				checkValidationData = new Customer[validationDataSize];
				for (int j=0; j<div*(k-1); j++){
					trainData[countTrain] = data[j];
					countTrain++;
				}
				for (int j=div*(k-1); j<dataSize; j++){
					validationData[countValidation] = data[j];
					checkValidationData[countValidation] = data[j];
					countValidation++;	
				}
			}
			else{
				trainDataSize = div*(k-1) + mod;
				validationDataSize = div;
				trainData = new Customer[trainDataSize];
				validationData = new Customer[validationDataSize];
				checkValidationData = new Customer[validationDataSize];
				for (int j=0; j<div*i; j++){
					trainData[countTrain] = data[j];
					countTrain++;
				}
				for (int j=div*i; j<div*(i+1); j++){
					validationData[countValidation] = data[j];
					checkValidationData[countValidation] = data[j];
					countValidation++;	
				}
				for (int j=div*(i+1); j<dataSize; j++){
					trainData[countTrain] = data[j];
					countTrain++;
				}
			}
			//----------------------MAKE ROOT NODE----------------------------
			root = makeNode();
			root->tempTrainData = trainData;
			root->size = trainDataSize;
			setThresholds(root);
			//------------------------GENERATE TREE---------------------------
			generateTree(root);
			cout << "Generate decision tree completed!" << endl;
			train(root, checkValidationData, validationDataSize);
			averageRaito += evaluateTrainQuality(checkValidationData, validationData, validationDataSize);
		}
		cout<<endl<< "-------Average Raito = "<<averageRaito/k<<"-----------"<<endl;
		
		if (argc == 4) {
			readStudyFile(argv[2], studyData, studyDataSize);
			train(root, studyData, studyDataSize);
			generateFile(argv[3],studyData, studyDataSize);
			cout << "See result in " << argv[3] << endl;
		}
		
	}
	else {
		printf("Syntax: CustomerClassification <dir_Historical File> <dir_Test File> <dir_Result + result_file_name>\n");
		printf("Eg: CustomerClassification C:\\train.dat D:\\test.dat E:\\result.txt\n");
	}	

	return 0;
}
