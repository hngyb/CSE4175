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

int dataword_size;
char *generator;
int generator_size;

vector<char> raw_data;
string binary_data;

void get_raw_data()
{
    /*
    input_file을 받는다.
    */
    char c;
    while (true)
    {
        c = input.get();
        if (input.fail())
        {
            break;
        }
        raw_data.push_back(c);
    }
}

void raw_data_to_binary()
{
    /*
    inpute_file을 ascii 코드로 변환 후, 2진수로 바꾸어 저장한다.
    */
    int ascii;
    string divident;
    for (vector<char>::iterator i = raw_data.begin(); i != raw_data.end(); i++)
    {
        ascii = *i;
        divident = bitset<8>(ascii).to_string();
        binary_data += divident;
    }
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

void dataword_to_codeword()
{
    /*
    dataword를 codeword로 변환 후, output file에 출력한다.
    */
    vector<string> divident;
    string encoded;
    string divb;
    string remainder;
    string codeword;
    string padding_num_bin;
    vector<string> split_codeword;

    int padding_num = 0;

    // generator - 1 크기만큼 0 bit를 divb에 저장한다.
    for (int i = 0; i < generator_size - 1; i++)
        divb += "0";

    size_t i = 0;
    size_t j;
    while (i < binary_data.size())
    { // dataword_size로 binary_data를 나누어 저장한다.
        string temp;
        for (j = i; j < i + dataword_size; j++)
        {
            temp += binary_data[j];
        }
        divident.push_back(temp);
        i = i + dataword_size;
    }

    for (i = 0; i < divident.size(); i++)
    { // dataword를 codeword로 변환한다.
        encoded = divident[i] + divb;
        remainder = modulo_2_div(encoded, string(generator));
        codeword += encoded.substr(0, dataword_size) + remainder;
    }

    padding_num = 8 - (codeword.length() % 8);
    if (padding_num != 0) // 패딩 비트를 추가한다.
    {
        for (int k = 0; k < padding_num; k++)
        {
            codeword.insert(0, 1, '0');
        }
    }

    // 패딩 비트를 출력한다.
    padding_num_bin = bitset<8>(padding_num).to_string();
    output << binaryToChar(padding_num_bin);

    i = 0;
    while (i < codeword.size())
    { // codeword를 바이트 단위로 나누어 저장한다.
        string temp;
        for (j = i; j < i + 8; j++)
        {
            temp += codeword[j];
        }
        split_codeword.push_back(temp);
        i = i + 8;
    }

    for (i = 0; i < split_codeword.size(); i++)
    { // codeword를 출력한다.
        output << binaryToChar(split_codeword[i]);
    }
}

int main(int argc, char *argv[])
{
    if (argc != 5) // 인자가 4개가 아닐 경우 에러메시지 출력
    {
        fprintf(stderr, "usage: ./crc_encoder input_file output_file generator dataword_size\n");
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
    dataword_size = atoi(argv[4]);
    if (dataword_size != 4 && dataword_size != 8)
    {
        fprintf(stderr, "dataword size must be 4 or 8.\n");
        exit(1);
    }

    generator = argv[3];
    generator_size = strlen(generator);

    get_raw_data(); // input_file 받기
    input.close();

    raw_data_to_binary();   // input_file을 이진수로 저장
    dataword_to_codeword(); // dataword를 codeword로 변환 및 출력
    output.close();

    return 0;
}