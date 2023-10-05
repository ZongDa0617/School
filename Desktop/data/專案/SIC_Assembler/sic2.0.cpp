#include<iostream>
#include <fstream>
#include <sstream>
#include<string>
#include<vector>
#include <iomanip>
#include<stdio.h>
using namespace std;

char registers[9][2]={//暫存器 
	{'A','0'},
	{'X','1'},
	{'L','2'},
	{'PC','8'},
	{'SW','9'},
	{'B','3'},
	{'S','4'},
	{'T','5'},
	{'F','6'},
	};
struct Data{//用來儲存程式資料 
	string loc=" ";
	string label=" ";
	string instruction=" ";
	string operand=" ";
	string objcode=" ";
	string format=" ";
	bool expand = false;
	int len=0;
};

struct op{
	string opcode=" ";
	string mnemonic=" ";
	string format=" ";
};
struct sym{
	string label=" ";
	string loc=" ";
};

vector<struct Data>SicCode;
vector<struct op>optab;
vector<struct sym>symtab;

void inputtxt(){
	
	ifstream opcode;
	string line;
	op op2;
	
	opcode.open("opcode.txt"); 
	while(getline(opcode,line)){
		string first,second,third;
		
		istringstream detail(line);
		
		detail>>first>>second>>third;
		op2.opcode=first;
		op2.format=second;
		op2.mnemonic=third;
		
		optab.push_back(op2);
	}
	opcode.close();
	
	ifstream SICCODE;
	SICCODE.open("SICCODE.txt");
	Data data1;
	
	while(getline(SICCODE,line)){
		string first,second,third=" ";
		
		istringstream detail(line);
		detail>>first>>second>>third;
		if(first=="."){
			continue;
		}
		if(third==" "){//判斷是不是只有接收到兩個，如果是要將兩個往後移，因為這代表沒有label
			third=second;
			second=first;
			first=" ";
		}
		data1.label=first;
		data1.instruction=second;
		data1.operand=third;
		SicCode.push_back(data1);
		
		sym sym1;
		if(first!=" "){//將label加進symtab 
			sym1.label=first;
			symtab.push_back(sym1);
		}
	}
	SICCODE.close();
}

string inttohex(string n){
	string hex;
	char b;
	int num=stoi(n);
	while(num){
		if(num%16<10){
			b=num%16+'0';
		}
		else{
			b=num%16+'A'-10;
		}
		hex.insert(hex.begin(),b);
		num/=16;
		
	}
	return hex;	
}
int hextoint(string hex){
	int num=0;
    for(int i=0;i<hex.size();i++){
        char t=hex[i];
        if(t>='0'&&t<='9'){
        	num=num*16+t-'0';
		}
        else{
        	num=num*16+t-'A'+10;
		}
    }
        return num;
}

string length="";
void pass1(){
	int loc=0;
	string loc2;
	
	for(int i=0;i<SicCode.size();i++){
		if(SicCode[i].instruction=="START"){
			loc=hextoint(SicCode[i].operand);
			
			loc2=SicCode[i].operand;
			while(loc2.size()<4){
				loc2="0"+loc2;
			}
			SicCode[i].loc=loc2; 
			
		}
		else if(SicCode[i].instruction=="END"){
			SicCode[i].loc=" ";
			break;
		}
		else if(SicCode[i].instruction=="BYTE"){
			if(SicCode[i].operand[0]=='C'){
				int len=SicCode[i].operand.size()-3;
				loc+=len;
				SicCode[i].len=len;
			}
			else if(SicCode[i].operand[0]=='X'){
				int len=SicCode[i].operand.size()-3;
				loc+=len/2;
				SicCode[i].len=len/2;
			}
			SicCode[i].format="3"; 
		}
		else if(SicCode[i].instruction=="RESB"){
			loc+=stoi(SicCode[i].operand);
			SicCode[i].len=stoi(SicCode[i].operand);
		}
		else if(SicCode[i].instruction=="RESW"){
			loc+=3;
			SicCode[i].len=3;
		}
		else if(SicCode[i].instruction=="BASE"){
			SicCode[i].loc=" ";
		}
		else if(SicCode[i].instruction[0]=='+'){
			loc+=4;
			SicCode[i].format="4";
			SicCode[i].expand=true;
			SicCode[i].len=4;
		}
		else{
			for(int j=0;j<optab.size();j++){
				if(SicCode[i].instruction.compare(optab[j].opcode)==0){
					if(optab[j].format.compare("3/4")==0){
						SicCode[i].format="3";
						loc+=3;
						SicCode[i].len=3;
					}
					else if(optab[j].format.compare("2")==0){
						SicCode[i].format="2";
						loc+=2;
						SicCode[i].len=2;
					}
					else if(optab[j].format.compare("1")==0){
						SicCode[i].format="1";
						loc+=1;
						SicCode[i].len=1;
					}
					
				}
			}
		}
		loc2=inttohex(to_string(loc));
		while(loc2.size()<4){//補0到四位數 
			loc2="0"+loc2;
		}
		SicCode[i+1].loc=loc2;		
	}
	length=loc2;
	
	for(int i=0;i<symtab.size();i++){
		for(int j=0;j<SicCode.size();j++){
			if(symtab[i].label.compare(SicCode[j].label)==0){
				symtab[i].loc=SicCode[j].loc;
			}
		}
	}
	
	ofstream out;
	out.open("Symtab.txt");
	for(unsigned int i = 0; i<symtab.size();i++){
        out<<left<<setw(10)<<symtab[i].label<<setw(10)<<symtab[i].loc<<endl;
    }
    out.close();
}

void pass2(){
	string Base="";
	for(int i=0;i<SicCode.size();i++){//算objcode 
		if(SicCode[i].instruction.compare("START")==0){
			SicCode[i].objcode=" ";
			continue;
		}
		else if(SicCode[i].instruction.compare("BASE")==0){
			SicCode[i].objcode=" ";
			
			for(int j=0;j<symtab.size();j++){
				if(SicCode[i].operand.compare(symtab[j].label)==0){
					Base=symtab[j].loc;
					break;
				}
			}
			continue;
		}
		else if(SicCode[i].instruction.compare("RESW")==0){
			SicCode[i].objcode=" ";
			continue;
		}
		else if(SicCode[i].instruction.compare("RESB")==0){
			SicCode[i].objcode=" ";
			continue;
		}
		else if(SicCode[i].instruction.compare("END")==0){
			SicCode[i].objcode=" ";
			break;
		}
		else{
			string objcode="";
			string PC="";
			string operand="";
			
			if(SicCode[i].expand){//+
				if(SicCode[i].operand[0]=='#'){
					string opstring=SicCode[i].instruction.substr(1,SicCode[i].instruction.size()-1);
					for(int j=0;j<optab.size();j++){
						if(opstring.compare(optab[j].opcode)==0){
							objcode=inttohex(to_string(hextoint(optab[j].mnemonic)+1));
							break;
						}
					}
					objcode+="10";
					string numstring=SicCode[i].operand.substr(1,SicCode[i].operand.size()-1);
					operand=inttohex(numstring);
				}	
				else{
					string opstring=SicCode[i].instruction.substr(1,SicCode[i].instruction.size()-1);
					for(int j=0;j<optab.size();j++){
						if(opstring.compare(optab[j].opcode)==0){
							objcode=inttohex(to_string(hextoint(optab[j].mnemonic)+3));
							break;
						}
					}
					objcode+="10";
					string labelstring=SicCode[i].operand;
					for(int j=0;j<symtab.size();j++){
							if(labelstring.compare(symtab[j].label)==0){
								operand=symtab[j].loc;
						}
					}
				}
				while(operand.size()<4){
						operand="0"+operand;
				}
				objcode+=operand;
			}
			else if(SicCode[i].format.compare("3")==0){
				if(SicCode[i].instruction.compare("BYTE")==0){
					if(SicCode[i].operand[0]=='X'){
						for(int k=0;k<SicCode[i].operand.size();k++){
							if(SicCode[i].operand[k]!='X' and SicCode[i].operand[k]!='\''){
								 objcode+=SicCode[i].operand[k];
							}
						}
					}
					else{
						for(int k=0;k<SicCode[i].operand.size();k++){
							if(SicCode[i].operand[k]!='C' and SicCode[i].operand[k]!='\''){
								 objcode+=inttohex(to_string(int(SicCode[i].operand[k])));
							}
						}
					}
				}
				else if(SicCode[i].operand.compare("")==0){
					for(int j=0;j<optab.size();j++){
						if(SicCode[i].instruction.compare(optab[j].opcode)==0){
							objcode=inttohex(to_string(hextoint(optab[j].mnemonic)+3));
							break;
						}
					}
					objcode+="0000";
				}
				else if(SicCode[i].operand[0]=='#'){//+1
					if(SicCode[i+1].loc!=" "){
						PC=SicCode[i+1].loc;
					}
					else{
						PC=SicCode[i+2].loc;
					}
					for(int j=0;j<optab.size();j++){
						if(SicCode[i].instruction.compare(optab[j].opcode)==0){
							objcode=inttohex(to_string(hextoint(optab[j].mnemonic)+1));
							break;
						}
					}
					if(objcode.size()<2){
						objcode="0"+objcode;
					}
					
					if(isdigit(SicCode[i].operand[1])){//立即定址後面是數字 
						string numstring="";
						for(int j=0;j<SicCode[i].operand.size();j++){
							if(SicCode[i].operand[j]!='#'){
								numstring+=SicCode[i].operand[j];
							}
						}
						while(numstring.size()<4){
							numstring="0"+numstring;
						}
						objcode+=numstring;
					}
					
					else{
						string labelstring=SicCode[i].operand.substr(1,SicCode[i].operand.size()-1);
						for(int j=0;j<symtab.size();j++){
							if(labelstring.compare(symtab[j].label)==0){
								operand=symtab[j].loc;
							}
						}
						string disp=inttohex(to_string(hextoint(operand)-hextoint(PC)));
						while(disp.size()<3){
							disp="0"+disp;
						}
						objcode+="2";
						objcode+=disp;
					}
				}
				else if(SicCode[i].operand[0]=='@'){//+2
					if(SicCode[i+1].loc!=" "){
						PC=SicCode[i+1].loc;
					}
					else{
						PC=SicCode[i+2].loc;
					}
					for(int j=0;j<optab.size();j++){
						if(SicCode[i].instruction.compare(optab[j].opcode)==0){
							objcode=inttohex(to_string(hextoint(optab[j].mnemonic)+2));
							break;
						}
					}
					string labelstring=SicCode[i].operand.substr(1,SicCode[i].operand.size()-1);
					for(int j=0;j<symtab.size();j++){
						if(labelstring.compare(symtab[j].label)==0){
							operand=symtab[j].loc;
						}
					}
					
					string disp=inttohex(to_string(hextoint(operand)-hextoint(PC)));
					while(disp.size()<3){
						disp="0"+disp;
					}
					objcode+="2";
					objcode+=disp;
				}
				else{//+3
					if(SicCode[i+1].loc!=" "){
						PC=SicCode[i+1].loc;
					}
					else{
						PC=SicCode[i+2].loc;
					}
					for(int j=0;j<optab.size();j++){
						if(SicCode[i].instruction.compare(optab[j].opcode)==0){
							objcode=inttohex(to_string(hextoint(optab[j].mnemonic)+3));
							break;
						}
					}
					while(objcode.size()<2){
						objcode="0"+objcode;
					}
					string labelstring="";
					int flagx=0;
					for(int j=0;j<SicCode[i].operand.size();j++){
						if(SicCode[i].operand[j]==','){
							flagx=1;
							break;
						}
						labelstring+=SicCode[i].operand[j];
					}
					
					for(int j=0;j<symtab.size();j++){
						if(labelstring.compare(symtab[j].label)==0){
							operand=symtab[j].loc;
						}
					}
					
					int numdisp = hextoint(operand)-hextoint(PC);
					string disp="";
					if(-2048<numdisp and numdisp<2047){//程式計數器相對定址模式
						if(numdisp<0){
							disp=inttohex(to_string(hextoint("FFF")+numdisp+1));//取二的補數
						} 
						else{
							disp=inttohex(to_string(numdisp));	
						}
						objcode+="2";
					}
					else{//基底相對定址模式 '
						disp=inttohex(to_string(hextoint(operand)-hextoint(Base)));
						if(flagx==1){
							objcode+="C";
						}
						else{
							objcode+="4";
						}
					} 
					while(disp.size()<3){
						disp="0"+disp;
					}
					objcode+=disp;
				} 
			}
			else if(SicCode[i].format.compare("2")==0){
				for(int j=0;j<optab.size();j++){
					if(SicCode[i].instruction.compare(optab[j].opcode)==0){
						objcode=optab[j].mnemonic;
						break;
					}
				}
				for(int j=0;j<SicCode[i].operand.size();j++){
					if(SicCode[i].operand[j]==','){
						continue;
					}
					
					for(int k=0;k<9;k++){
						if(SicCode[i].operand[j]==registers[k][0]){
							objcode+=registers[k][1];
						}
					}
				}
				if(objcode.size()<4){
						objcode+="0";
				}
			}
			else if(SicCode[i].format.compare("1")==0){
				for(int j=0;j<optab.size();j++){
					if(SicCode[i].instruction.compare(optab[j].opcode)==0){
						objcode=optab[j].mnemonic;
						break;
					}
				}	
			}
			SicCode[i].objcode=objcode;
		}
	}
	
	
	ofstream out;
	out.open("ObjectProgram.txt");
	//H 
	out<<"H"<<left<<setw(6)<<SicCode[0].label<<"^"
			<<right<<setw(6)<<setfill('0')<<SicCode[0].loc<<"^"
				  <<setw(6)<<setfill('0')<<length<<endl;
	
	//T 
	int Tlen=0;
	string head=SicCode[1].loc;
	vector<string> Tstring;
	for(int i=1;i<SicCode.size();i++){
		int codelen=SicCode[i].len;
		if(Tlen+codelen>30){
			out<<"T^"<<setw(6)<<setfill('0')<<head<<"^" 
					<<setw(2)<<setfill('0')<<inttohex(to_string(Tlen))<<"^";
			for(int j=0;j<Tstring.size();j++){
				out<<Tstring[j]<<"^";
			}
			out<<endl;
			Tstring.clear();
			head=SicCode[i].loc;
			Tlen=0;
		}
		if(SicCode[i].objcode.compare(" ")!=0){
			Tlen+=codelen;
			Tstring.push_back(SicCode[i].objcode);
		} 
	}
	if(!Tstring.empty()){
		out<<"T^"<<setw(6)<<setfill('0')<<head<<"^" 
				<<setw(2)<<setfill('0')<<inttohex(to_string(Tlen))<<"^";
		for(int j=0;j<Tstring.size();j++){
			out<<Tstring[j]<<"^";
		}
		out<<endl;
	}
	//M
	for(int i=0;i<SicCode.size();i++){
		if(SicCode[i].expand){
			if(SicCode[i].operand[0]=='#'){
				continue;
			} 
			out<<"M^"<<setw(6)<<setfill('0')
				<<inttohex(to_string(hextoint(SicCode[i].loc)+1))<<"^"
						<<setw(2)<<"05"<<endl;
		}
	}
	//E
	out<<"E^"<<setw(6)<<setfill('0')<<SicCode[1].loc; 
	
	out.close();
	
}
int main(){
	inputtxt();
	pass1();
	pass2();
	ofstream siccode4;
	siccode4.open("SicCode2.txt");
	for(unsigned int i = 0; i<SicCode.size();i++){
        siccode4<<left<<setw(10)<<SicCode[i].label<<setw(10)<<SicCode[i].instruction
             <<setw(10)<<SicCode[i].operand<<setw(10)<<SicCode[i].loc
			 <<setw(10)<<SicCode[i].objcode<<endl;
    }
    siccode4.close();
    
    return 0;
}
