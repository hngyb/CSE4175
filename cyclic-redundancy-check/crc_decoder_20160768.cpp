#include "iostream"
#include "fstream"
#include "string.h"
#include "vector"
#include "stdlib.h"
#include "bitset"
#include "sstream"
using namespace std;

ifstream input;
ofstream output;
ofstream result;

int dataword_size;
char *generator;
int generator_size;

vector<char> input_data;
string encoded_data;
string dataword;

int padding_num;
int codeword_num = 0;
int err_num = 0;

void read_input()
{
    /*
    encode된 파일을 읽는다.
    */
    char c;
    int ascii;
    string temp;
    while (true)
    {
        c = input.get();
        if (input.fail())
        {
            break;
        }
        input_data.push_back(c);
    }
    for (vector<char>::iterator i = input_data.begin(); i != input_data.end(); i++)
    {
        ascii = *i;
        temp = bitset<8>(ascii).to_string();
        encoded_data += temp;
    }
    padding_num = strtol(encoded_data.substr(0, 8).c_str(), 0, 2); // 패딩의 크기를 알아낸다.
    encoded_data = encoded_data.substr(8 + padding_num);           // 패딩을 제거한다.
}

string modulo_2_div(string encoded, string generator)
{
    /*
    modulo-2 division을 진행한다.
    */
    size_t i;
    int j;
    string remainder;

    i = 0;
    while (i <= (encoded.length() - generator_size))
    {
        for (j = 0; j < generator_size; j++)
        {
            if (encoded[i + j] == generator[j])
                encoded[i + j] = '0';
            else
                encoded[i + j] = '1';
        }
        while (i < encoded.length() && encoded[i] == '0')
            i++;
    }
    remainder = encoded.substr(dataword_size);
    return remainder;
}

void errCheck()
{
    /*
    codeword에 오류가 있는지 확인한 후, result file에 출력한다.
    */
    vector<string> divident;
    string temp;
    string remainder;

    size_t i = 0;
    size_t j;
    while (i < encoded_data.size())
    {
        string temp;
        for (j = i; j < i + dataword_size + generator_size - 1; j++)
        {
            temp += encoded_data[j];
        }
        divident.push_back(temp);
        i = i + dataword_size + generator_size - 1;
    }

    for (i = 0; i < divident.size(); i++)
    {
        remainder = modulo_2_div(divident[i], generator);
        if (strtol(remainder.c_str(), 0, 2) != 0)
        {
            err_num++;
        }
        dataword += divident[i].substr(0, dataword_size);
        codeword_num++;
    }
    result << codeword_num << ' ' << err_num;
}

string binaryToChar(string binary)
{
    /*
    이진수를 문자로 변환한다.
    */
    string c;
    stringstream sstream(binary);
    bitset<8> bit;

    sstream >> bit;
    c = char(bit.to_ulong());

    return c;
}

void codeword_to_dataword()
{
    /*
    codeword를 dataword로 복원한다.
    */
    vector<string> split_dataword;

    size_t i = 0;
    size_t j;
    while (i < dataword.size())
    {
        string temp;
        for (j = i; j < i + 8; j++)
        {
            temp += dataword[j];
        }
        split_dataword.push_back(temp);
        i = i + 8;
    }

    for (i = 0; i < split_dataword.size(); i++)
    {
        output << binaryToChar(split_dataword[i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 6) // 인자가 5개가 아닐 경우 에러메시지 출력
    {
        fprintf(stderr, "usage: ./crc_decoder input_file output_file result_file generator dataword_size\n");
        exit(1);
    }

    // 인자 에러 체크
    input.open(argv[1], ios::binary);
    if (input.fail())
    {
        fprintf(stderr, "input file open error.\n");
        exit(1);
    }
    output.open(argv[2], ios::binary);
    if (output.fail())
    {
        fprintf(stderr, "output file open error.\n");
        exit(1);
    }
    result.open(argv[3]);
    if (!result)
    {
        fprintf(stderr, "result file open error.\n");
        exit(1);
    }
    dataword_size = atoi(argv[5]);
    if (dataword_size != 4 && dataword_size != 8)
    {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1);
    }

    generator = argv[4];
    generator_size = strlen(generator);

    read_input(); // encode된 파일을 읽는다.
    input.close();

    errCheck(); // codeword의 에러를 확인한다.
    result.close();

    codeword_to_dataword(); // codeword를 dataword로 복원하여 출력한다.
    output.close();

    return 0;
}