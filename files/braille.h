/**
 * BrailleUtils - A series of functions that can easily handle Braille
 * Comments are mostly in Simplified Chinese
 * If you want other languages of the comments, give this header file to AI, they're really smart nowadays
 * This is not a standard C++ header (how can it be?)
 * This header highly relies on modern C++, and may (almost must) cause glitches on C
 * Made by hqc8848 <hqc8848@gmail.com>, some code maybe written by AI
 * 
 * BrailleUtils —— 一组便于盲文处理的实用函数
 * 注释主要以简体中文撰写
 * 如果你需要其他语言的注释，把这个头文件丢给AI就好，它们现如今已经很聪明了
 * 这不是一个标准的 C++ 头文件（怎么可能会是呢）
 * 这个头文件高度依赖于现代 C++，在 C 语言中可能（几乎是肯定）会导致故障
 * 由 hqc8848 <hqc8848@gmail.com> 制作，部分代码可能由人工智能撰写
 */

#ifndef _BRAILLEUTILS_INCLUDED_

#define _BRAILLEUTILS_INCLUDED_
#include <algorithm>
#include <string>

namespace braille {
	const std::string DEFAULT_BRACKETS = "([{<)]}>";

	/**
	 * Transfer dots string to single Braille - 将点位字符串转换为单个盲文
	 * 参数：
	 * `dots` ：点位字符串，非法的点位字符将被忽略
	 * 
	 * 点位字符串标志凸起的盲文点位，建议但不强制排序
	 * 单个或多个盲文按照 UTF-8 可变编码输出，可能不适用于 ASCII-only 环境
	 * 以下按照相对于实际显示的位置标注各个盲文点位的编号，用于点位字符串：
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 */
	std::string d2b (const std::string& dots) {
		std::string braille = "";
		unsigned char raised = 0;
		for (const auto& d : dots) if ('1' <= d && d <= '8') raised += (1 << (d - '1'));
		braille.push_back(0xE2);
		braille.push_back(0xA0 + (raised >> 6));
		braille.push_back(0x80 + (raised & 0x3F));
		return braille;
	}

	/**
	 * Transfer single Braille to dots string - 将单个盲文转换为点位字符串
	 * 参数：
	 * `braille` ：单个盲文，非法的盲文将被忽略
	 * 
	 * 点位字符串标志凸起的盲文点位，建议但不强制排序
	 * 单个或多个盲文按照 UTF-8 可变编码输出，可能不适用于 ASCII-only 环境
	 * 以下按照相对于实际显示的位置标注各个盲文点位的编号，用于点位字符串：
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 */
	std::string b2d (const std::string& braille) {
		std::string dots = "";
		unsigned char raised = ((braille[1] & 0x0F) << 6) | (braille[2] & 0x3F);
		for (int i = 0; i < 8; i++) if ((raised >> i) & 1) dots.push_back(i + '1');
		return dots;
	}

	/**
	 * Transfer batch dots string to multiple Brailles - 将批量点位字符串转换为多个盲文
	 * 参数：
	 * `batch_dots` ：批量点位字符串，非法的点位字符串中的非法字符将被忽略
	 * `brackets` （可选）：包裹符号字符串，长度需为偶数，若长度为奇数，最后字符将被忽略。该值默认为 braille::DEFAULT_BRACKETS
	 * 
	 * 点位字符串标志凸起的盲文点位，建议但不强制排序
	 * 单个或多个盲文按照 UTF-8 可变编码输出，可能不适用于 ASCII-only 环境
	 * 以下按照相对于实际显示的位置标注各个盲文点位的编号，用于点位字符串：
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 * 
	 * 批量点位字符串即多段点位字符串的组合，每段点位字符串必须以指定的包裹符号包裹，否则视为非法，按原样返回
	 * 包裹符号字符串即包裹符号的指定合集，分为前后两段，前段表示可用的包裹开头，后段表示可用的包裹结尾。包裹开头和包裹结尾不要求但建议在前后两段中一一对应
	 */
	std::string batch_d2b (const std::string& batch_dots, const std::string& brackets = braille::DEFAULT_BRACKETS) {
		std::string brailles = "", dot = "";
		const unsigned long long brackets_halflen = brackets.size() / 2;
		bool in_bracket = false;
		for (const auto& bd : batch_dots) {
			if (!(in_bracket)) {
				if (std::find(brackets.begin(), brackets.begin()+brackets_halflen, bd) != brackets.begin()+brackets_halflen) in_bracket = true;
				else brailles.push_back(bd);
			} else {
				if (std::find(brackets.begin()+brackets_halflen, brackets.begin()+brackets_halflen*2, bd) != brackets.begin()+brackets_halflen*2) {
					brailles.append(braille::d2b(dot));
					dot = "";
					in_bracket = false;
				} else dot.push_back(bd);
			}
		} return brailles;
	}

	/**
	 * Transfer multiple Brailles to batch dots string - 将多个盲文转换为批量点位字符串
	 * 参数：
	 * `brailles` ：多个盲文，非法盲文将按原样返回
	 * `brackets` （可选）：包裹符号字符串，长度需为偶数，若长度为奇数，最后字符将被忽略。该值默认为 braille::DEFAULT_BRACKETS
	 * 
	 * 点位字符串标志凸起的盲文点位，建议但不强制排序
	 * 单个或多个盲文按照 UTF-8 可变编码输出，可能不适用于 ASCII-only 环境
	 * 以下按照相对于实际显示的位置标注各个盲文点位的编号，用于点位字符串：
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 * 
	 * 批量点位字符串即多段点位字符串的组合，每段点位字符串以指定的包裹符号包裹，此处默认使用包裹符号字符串中出现的第一组可用包裹符号
	 * 包裹符号字符串即包裹符号的指定合集，分为前后两段，前段表示可用的包裹开头，后段表示可用的包裹结尾。包裹开头和包裹结尾不要求但建议在前后两段中一一对应
	 */
	std::string batch_b2d (const std::string& brailles, const std::string& brackets = braille::DEFAULT_BRACKETS) {
		const unsigned long long brackets_halflen = brackets.size() / 2;
		if (brailles.size() < 3 || brackets_halflen == 0) return brailles;
		std::string batch_dots = "";
		for (unsigned long long i = 0; i < brailles.size(); i++) {
			const unsigned char first = static_cast<unsigned char>(brailles[i]);
			const bool is_braille = i + 2 < brailles.size()
				&& first == 0xE2
				&& static_cast<unsigned char>(brailles[i + 1]) >= 0xA0
				&& static_cast<unsigned char>(brailles[i + 1]) <= 0xA3
				&& static_cast<unsigned char>(brailles[i + 2]) >= 0x80
				&& static_cast<unsigned char>(brailles[i + 2]) <= 0xBF;
			if (is_braille) {
				batch_dots.push_back(brackets[0]);
				batch_dots.append(b2d(brailles.substr(i, 3)));
				batch_dots.push_back(brackets[brackets_halflen]);
				i += 2;
			} else batch_dots.push_back(brailles[i]);
		} return batch_dots;
	}

	/**
	 * 字节加密类，按字节加密，最基础
	 * 
	 * 成员：
	 * `encode(const std::string& plain)` ：调用 braille::d2b() 逐个字节转换成盲文
	 * `decode(const std::string& cipher)` ：调用 braille::b2d() 逐个盲文还原为字节，非法字节序列按原样返回
	 */
	class Byte {
		public:
			std::string target;
			Byte(std::string x) : target(x) {}

			inline std::string encode (const std::string& plain) const {
				std::string cipher = "";
				for (const auto& p : plain) {
					std::string single = "";
					for (short i = 0; i < 8; i++)
						if (static_cast<unsigned char>(p) >> i & 1) single.push_back('1' + i);
					cipher.append(d2b(single));
				} return cipher;
			} inline std::string decode (const std::string& cipher) const {
				std::string plain = "";
				for (unsigned long long i = 0; i < cipher.size();) {
					const bool is_braille = i + 2 < cipher.size()
						&& static_cast<unsigned char>(cipher[i]) == 0xE2
						&& static_cast<unsigned char>(cipher[i + 1]) >= 0xA0
						&& static_cast<unsigned char>(cipher[i + 1]) <= 0xA3
						&& static_cast<unsigned char>(cipher[i + 2]) >= 0x80
						&& static_cast<unsigned char>(cipher[i + 2]) <= 0xBF;
					if (!is_braille) {
						plain.push_back(cipher[i++]);
						continue;
					} const std::string dots = b2d(cipher.substr(i, 3));
					unsigned char decoded = 0;
					for (const auto& dot : dots) decoded |= 1 << (dot - '1');
					plain.push_back(static_cast<char>(decoded));
					i += 3;
				} return plain;
			} inline std::string encode () const {
				return encode(target);
			} inline std::string decode () const {
				return decode(target);
			}
	};
}

#endif
