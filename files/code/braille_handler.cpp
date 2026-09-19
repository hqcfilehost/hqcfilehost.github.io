/**
 * braille_handler.cpp
 *
 * 输入格式：
 *   程序从当前工作目录的 ./input.txt 读取全部内容；整个文件作为一个
 *   字符串处理，换行符也会参与转换，不按行切分，也没有固定的行号范围。
 *   文件按二进制方式读取，因此可以包含空字节；输入文件末尾的换行符
 *   不会被自动删除。
 *
 *   mode = byte-encode：
 *     输入中的每一个字节（包括 ASCII 字符、UTF-8 字节、空字节等）对应
 *     一个盲文字符。这里的盲文只是字节的可逆表示，不代表英语含义。
 *   mode = byte-decode：
 *     输入中的每个合法 UTF-8 盲文字符（连续 3 个字节，范围为
 *     E2 A0 80 至 E2 A3 BF）还原为一个字节；其他字符原样保留。
 *
 *   mode = english-encode：
 *     输入为英文文本、数字和 braille.h 支持的标点。a-z 按一级英语盲文
 *     编码，A-Z 前面增加大写标志，数字前面增加数字标志；连续数字中的
 *     小数点和逗号也按数字规则处理。未定义的字符原样保留。
 *   mode = english-decode：
 *     输入为英语盲文字符及其大小写/数字标志，解码为英文、数字和标点；
 *     无法识别的内容原样保留。
 *
 *   mode = d2b：
 *     输入是一个点位字符串，只识别字符 '1' 到 '8'，每个数字代表一个
 *     凸起的盲文点，位置如下：
 *
 *       1 4
 *       2 5
 *       3 6
 *       7 8
 *
 *     例如：145 表示第 1、4、5 点凸起，其他字符会被忽略。该模式只把
 *     整个输入作为一个盲文字符的点位集合。
 *   mode = b2d：
 *     输入必须是一个 UTF-8 盲文字符（3 个字节），输出其凸起点位，例如
 *     盲文字符对应的点位可能输出为 145；非法或长度不为 3 的输入输出空串。
 *
 *   mode = batch-d2b：
 *     输入是普通文本与被括号包围的点位组的混合内容。默认括号为 (),
 *     []、{} 和 <> 也可作为括号；每个括号内的 '1' 到 '8' 表示一个盲文
 *     字符。例如：[145](12) 表示两个盲文字符。未被正确括起的点位组
 *     按原样保留。
 *   mode = batch-b2d：
 *     输入是普通文本与 UTF-8 盲文字符的混合内容；每个盲文字符会转换为
 *     默认括号包围的点位组，例如一个点位为 145 的盲文会输出 (145)。
 *     非法盲文字符和普通文本原样保留。
 *
 * 输出格式：
 *   程序将转换结果以二进制方式写入当前工作目录的 ./output.txt。输出是
 *   对 ./input.txt 全部内容的一次转换，不自动追加换行，也不修改输入文件。
 *   对于 encode/decode 成对模式，先使用 encode，再把生成的 output.txt
 *   复制为 input.txt 并使用对应 decode，通常可以恢复原始内容。
 *
 * 使用方式：
 *   ./braille_handler <mode>
 *   可用模式见程序运行时显示的 Usage 信息。
 *
 * 本文件由 AI 生成。
 */

#if !defined(__cplusplus) || __cplusplus < 201103L
#error "This file requires C++11 or a newer C++ standard"
#endif

#include "braille.h"

#include <fstream>
#include <iostream>
#include <iterator>
#include <string>

namespace {

typedef std::string (*Transform)(const std::string&);

std::string read_file(const std::string& path) {
    std::ifstream input(path.c_str(), std::ios::in | std::ios::binary);
    if (!input) {
        throw std::ios_base::failure("cannot open input file: " + path);
    }
    return std::string(std::istreambuf_iterator<char>(input),
                       std::istreambuf_iterator<char>());
}

void write_file(const std::string& path, const std::string& value) {
    std::ofstream output(path.c_str(), std::ios::out | std::ios::binary |
                                      std::ios::trunc);
    if (!output) {
        throw std::ios_base::failure("cannot open output file: " + path);
    }
    output.write(value.data(), static_cast<std::streamsize>(value.size()));
    if (!output) {
        throw std::ios_base::failure("cannot write output file: " + path);
    }
}

std::string byte_encode(const std::string& value) {
    return braille::Byte().encode(value);
}

std::string byte_decode(const std::string& value) {
    return braille::Byte().decode(value);
}

std::string english_encode(const std::string& value) {
    return braille::English().encode(value);
}

std::string english_decode(const std::string& value) {
    return braille::English().decode(value);
}

std::string dots_encode(const std::string& value) {
    return braille::d2b(value);
}

std::string dots_decode(const std::string& value) {
    return braille::b2d(value);
}

std::string batch_dots_encode(const std::string& value) {
    return braille::batch_d2b(value);
}

std::string batch_dots_decode(const std::string& value) {
    return braille::batch_b2d(value);
}

Transform select_transform(const std::string& mode) {
    if (mode == "byte-encode") return byte_encode;
    if (mode == "byte-decode") return byte_decode;
    if (mode == "english-encode") return english_encode;
    if (mode == "english-decode") return english_decode;
    if (mode == "d2b") return dots_encode;
    if (mode == "b2d") return dots_decode;
    if (mode == "batch-d2b") return batch_dots_encode;
    if (mode == "batch-b2d") return batch_dots_decode;
    return 0;
}

void print_usage(const char* program) {
    std::cerr
        << "Usage: " << program << " <mode>\n"
        << "Reads ./input.txt and writes ./output.txt.\n\n"
        << "Modes:\n"
        << "  byte-encode    Byte::encode\n"
        << "  byte-decode    Byte::decode\n"
        << "  english-encode English::encode\n"
        << "  english-decode English::decode\n"
        << "  d2b            braille::d2b\n"
        << "  b2d            braille::b2d\n"
        << "  batch-d2b      braille::batch_d2b\n"
        << "  batch-b2d      braille::batch_b2d\n";
}

}  // namespace

int main(int argc, char* argv[]) {
    if (argc != 2) {
        print_usage(argv[0]);
        return 2;
    }

    const Transform transform = select_transform(argv[1]);
    if (!transform) {
        std::cerr << "Unknown mode: " << argv[1] << "\n";
        print_usage(argv[0]);
        return 2;
    }

    try {
        write_file("./output.txt", transform(read_file("./input.txt")));
    } catch (const std::ios_base::failure& error) {
        std::cerr << "braille_handler: " << error.what() << "\n";
        return 1;
    }
    return 0;
}