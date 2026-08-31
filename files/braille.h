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

#pragma once

#ifndef _BRAILLEUTILS_INCLUDED_
#define _BRAILLEUTILS_INCLUDED_
#include <algorithm>
#include <cctype>
#include <cstdint>
#include <map>
#include <string>
#include <utility>
#include <vector>

namespace braille {
	static const std::string DEFAULT_BRACKETS = "([{<)]}>";

	// 实现细节，不建议使用，未来可能发生变动
	namespace detail {
		inline bool is_braille_at (const std::string& value, std::string::size_type pos) {
			if (pos + 2 >= value.size()) return false;
			const std::uint8_t first = static_cast<std::uint8_t>(static_cast<unsigned char>(value[pos]));
			const std::uint8_t second = static_cast<std::uint8_t>(static_cast<unsigned char>(value[pos + 1]));
			const std::uint8_t third = static_cast<std::uint8_t>(static_cast<unsigned char>(value[pos + 2]));
			return first == 0xE2 && second >= 0xA0 && second <= 0xA3 && third >= 0x80 && third <= 0xBF;
		}
	}

	/**
	 * Transfer dots string to single Braille - 将点位字符串转换为单个盲文
	 * 参数：
	 * `dots` ：点位字符串，非法的点位字符将被忽略
	 * 
	 * 点位字符串标志凸起的盲文点位，建议但不强制排序
	 * 单个或多个盲文按照 UTF-8 可变编码输出，可能不适用于 ASCII-only 环境
	 * 以下按照相对于实际显示的位置标注各个盲文点位的编号，用于点位字符串：
	 * ```plaintext
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * ```
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 */
	inline std::string d2b (const std::string& dots) {
		std::string braille;
		std::uint8_t raised = 0;
		for (const auto& d : dots) if ('1' <= d && d <= '8') raised += (1 << (d - '1'));
		braille.push_back(static_cast<char>(0xE2));
		braille.push_back(static_cast<char>(0xA0 + (raised >> 6)));
		braille.push_back(static_cast<char>(0x80 + (raised & 0x3F)));
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
	 * ```plaintext
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * ```
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 */
	inline std::string b2d (const std::string& braille) {
		if (braille.size() != 3 || !detail::is_braille_at(braille, 0)) return "";
		std::string dots;
		const std::uint8_t second = static_cast<std::uint8_t>(static_cast<unsigned char>(braille[1]));
		const std::uint8_t third = static_cast<std::uint8_t>(static_cast<unsigned char>(braille[2]));
		const std::uint8_t raised = static_cast<std::uint8_t>(((second & 0x03) << 6) | (third & 0x3F));
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
	 * ```plaintext
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * ```
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 * 
	 * 批量点位字符串即多段点位字符串的组合，每段点位字符串必须以指定的包裹符号包裹，否则视为非法，按原样返回
	 * 包裹符号字符串即包裹符号的指定合集，分为前后两段，前段表示可用的包裹开头，后段表示可用的包裹结尾。包裹开头和包裹结尾不要求但建议在前后两段中一一对应
	 */
	inline std::string batch_d2b (const std::string& batch_dots, const std::string& brackets = braille::DEFAULT_BRACKETS) {
		std::string brailles, dot;
		const std::string::size_type brackets_halflen = brackets.size() / 2;
		if (brackets_halflen == 0) return batch_dots;
		bool in_bracket = false;
		std::string::size_type group_start = 0;
		for (const auto& bd : batch_dots) {
			if (!(in_bracket)) {
				if (std::find(brackets.begin(), brackets.begin()+brackets_halflen, bd) != brackets.begin()+brackets_halflen) {
					in_bracket = true;
					group_start = brailles.size();
				} else brailles.push_back(bd);
			} else {
				if (std::find(brackets.begin()+brackets_halflen, brackets.begin()+brackets_halflen*2, bd) != brackets.begin()+brackets_halflen*2) {
					brailles.append(braille::d2b(dot));
					dot = "";
					in_bracket = false;
				} else dot.push_back(bd);
			}
		} if (in_bracket) {
			brailles.erase(group_start);
			brailles.append(batch_dots.substr(batch_dots.size() - dot.size() - 1));
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
	 * ```plaintext
	 * 1  4
	 * 2  5
	 * 3  6
	 * 7  8
	 * ```
	 * 注：前六位编号亦被标准编号 GF0019-2018，即《国家通用盲文方案》使用
	 * 
	 * 批量点位字符串即多段点位字符串的组合，每段点位字符串以指定的包裹符号包裹，此处默认使用包裹符号字符串中出现的第一组可用包裹符号
	 * 包裹符号字符串即包裹符号的指定合集，分为前后两段，前段表示可用的包裹开头，后段表示可用的包裹结尾。包裹开头和包裹结尾不要求但建议在前后两段中一一对应
	 */
	inline std::string batch_b2d (const std::string& brailles, const std::string& brackets = braille::DEFAULT_BRACKETS) {
		const std::string::size_type brackets_halflen = brackets.size() / 2;
		if (brailles.size() < 3 || brackets_halflen == 0) return brailles;
		std::string batch_dots;
		for (std::string::size_type i = 0; i < brailles.size(); i++) {
			const bool is_braille = braille::detail::is_braille_at(brailles, i);
			if (is_braille) {
				batch_dots.push_back(brackets[0]);
				batch_dots.append(b2d(brailles.substr(i, 3)));
				batch_dots.push_back(brackets[brackets_halflen]);
				i += 2;
			} else batch_dots.push_back(brailles[i]);
		} return batch_dots;
	}

	/**
	 * 字节编码类，按字节编码，最基础
	 * 
	 * 成员：
	 * `encode(const std::string& plain)` ：调用 braille::d2b() 逐个字节转换成盲文
	 * `decode(const std::string& cipher)` ：调用 braille::b2d() 逐个盲文还原为字节，非法字节序列按原样返回
	 * `set_target(const std::string& tgt)` ：设置待处理的目标字符串
	 * `get_target()` ：获取当前目标字符串
	 * `target` ：目标字符串，可通过无参数的encode()或decode()直接处理
	 */
	class Byte {
		public:
			std::string target;
			Byte() = default;
			explicit Byte(const std::string& x) : target(x) {}
			explicit Byte(std::string&& x) noexcept : target(std::move(x)) {}
			Byte(const Byte&) = default;
			Byte(Byte&&) noexcept = default;
			Byte& operator = (const Byte&) = default;
			Byte& operator = (Byte&&) noexcept = default;
			~Byte() = default;

			inline void set_target (const std::string& tgt) { target = tgt; }
			inline const std::string& get_target () const noexcept { return target; }

			inline std::string encode (const std::string& plain) const {
				std::string cipher;
				if (plain.size() <= plain.max_size() / 3) cipher.reserve(plain.size() * 3);
				for (const auto& p : plain) {
					const std::uint8_t byte = static_cast<std::uint8_t>(static_cast<unsigned char>(p));
					cipher.push_back(static_cast<char>(0xE2));
					cipher.push_back(static_cast<char>(0xA0 | (byte >> 6)));
					cipher.push_back(static_cast<char>(0x80 | (byte & 0x3F)));
				} return cipher;
			}

			inline std::string decode (const std::string& cipher) const {
				std::string plain;
				plain.reserve(cipher.size() / 3);
				for (std::string::size_type i = 0; i < cipher.size();) {
					const bool is_braille = braille::detail::is_braille_at(cipher, i);
					if (!is_braille) {
						plain.push_back(cipher[i++]);
						continue;
					} const std::uint8_t second = static_cast<std::uint8_t>(static_cast<unsigned char>(cipher[i + 1]));
					const std::uint8_t third = static_cast<std::uint8_t>(static_cast<unsigned char>(cipher[i + 2]));
					const std::uint8_t decoded = static_cast<std::uint8_t>(((second & 0x03) << 6) | (third & 0x3F));
					plain.push_back(static_cast<char>(decoded));
					i += 3;
				} return plain;
			}

			inline std::string encode () const { return encode(target); }
			inline std::string decode () const { return decode(target); }
	};

	/**
	 * 英语编码类，按字母、标点、组合音素等编码，支持底层一级英语盲文，二级盲文的常见合并词对照见braille::English::usage;
	 * 
	 * 成员：
	 * `encode(const std::string& plain)` ：将英文、数字和标点转换为盲文，大小写和数字按照英语盲文规则添加对应符号
	 * `decode(const std::string& cipher)` ：将盲文还原为英文、数字和标点，非法盲文按原样返回
	 * `add_mapping(const std::string& plain, const std::string& braille)` ：添加或覆盖一组自定义明文与盲文对照
	 * `set_target(const std::string& tgt)` ：设置待处理的目标字符串
	 * `get_target()` ：获取当前目标字符串
	 * `target` ：目标字符串，可通过无参数的encode()或decode()直接处理
	 */
	class English {
		private:
			std::map<std::string, std::string> plain_to_braille;
			std::map<std::string, std::string> braille_to_plain;

			inline void add_default (const std::string& plain, const std::string& dots) {
				add_mapping(plain, braille::d2b(dots));
			}

			inline void add_defaults () {
				const std::string letters[26] = {
					"1", "12", "14", "145", "15", "124", "1245", "125", "24", "245",
					"13", "123", "134", "1345", "135", "1234", "12345", "1235", "234", "2345",
					"136", "1236", "2456", "1346", "13456", "1356"
				}; for (int i = 0; i < 26; i++) add_default(std::string(1, static_cast<char>('a' + i)), letters[i]);

				const std::pair<const char*, const char*> contractions[] = {
					{"for", "123456"}, {"of", "12356"}, {"the", "2346"}, {"with", "23456"},
					{"ch", "16"}, {"gh", "126"}, {"sh", "146"}, {"th", "1456"}, {"wh", "156"},
					{"ed", "1246"}, {"er", "12456"}, {"ou", "1256"}, {"ow", "246"}, {"and", "12346"},
					{"st", "34"}, {"ar", "345"}, {"in", "35"}, {"ing", "346"}
				}; for (const auto& contraction : contractions) add_default(contraction.first, contraction.second);

				const std::pair<const char*, const char*> punctuation[] = {
					{",", "2"}, {";", "23"}, {":", "25"}, {".", "256"}, {"!", "235"},
					{"?", "236"}, {"'", "3"}, {"-", "36"}, {"(", "2356"}, {")", "2356"},
					{"\"", "236"}, {"/", "34"}
				}; for (const auto& mark : punctuation) add_default(mark.first, mark.second);
			}

			inline std::string find_plain (const std::string& plain, std::string::size_type pos) const {
				std::string result;
				for (const auto& item : plain_to_braille) {
					if (item.first.size() <= result.size() || pos + item.first.size() > plain.size()) continue;
					bool matches = true;
					for (std::string::size_type i = 0; i < item.first.size(); i++) {
						if (static_cast<char>(std::tolower(static_cast<unsigned char>(plain[pos + i]))) != item.first[i]) {
							matches = false;
							break;
						}
					} if (matches) result = item.first;
				} return result;
			}

			inline bool is_number_character (char value) const {
				return ('0' <= value && value <= '9') || value == '.' || value == ',';
			}

		public:
			std::string target;

			English() { add_defaults(); }
			explicit English(const std::string& x) : target(x) { add_defaults(); }
			explicit English(std::string&& x) noexcept : target(std::move(x)) { add_defaults(); }
			English(const English&) = default;
			English(English&&) noexcept = default;
			English& operator = (const English&) = default;
			English& operator = (English&&) noexcept = default;
			~English() = default;

			inline void set_target (const std::string& tgt) { target = tgt; }
			inline const std::string& get_target () const noexcept { return target; }

			/**
			 * 添加或覆盖一组明文与盲文对照，`braille` 必须是一个 UTF-8 盲文字符。
			 * 明文匹配不区分大小写，大小写仍由大写符号表示。
			 */
			inline void add_mapping (const std::string& plain, const std::string& braille) {
				if (plain.empty() || braille.size() != 3 || !detail::is_braille_at(braille, 0)) return;
				std::string normalized = plain;
				for (auto& character : normalized)
					character = static_cast<char>(std::tolower(static_cast<unsigned char>(character)));
				const auto old_plain = braille_to_plain.find(braille);
				if (old_plain != braille_to_plain.end()) plain_to_braille.erase(old_plain->second);
				const auto old_braille = plain_to_braille.find(normalized);
				if (old_braille != plain_to_braille.end()) braille_to_plain.erase(old_braille->second);
				plain_to_braille[normalized] = braille;
				braille_to_plain[braille] = normalized;
			}

			inline std::string encode (const std::string& plain) const {
				std::string cipher;
				for (std::string::size_type i = 0; i < plain.size();) {
					const char current = plain[i];
					if (std::isdigit(static_cast<unsigned char>(current))) {
						cipher += braille::d2b("3456");
						while (i < plain.size() && is_number_character(plain[i])) {
							const char digit = plain[i];
							if (digit == '.' || digit == ',') {
								const auto punctuation = plain_to_braille.find(std::string(1, digit));
								if (punctuation != plain_to_braille.end()) cipher += punctuation->second;
							} else {
								const char letter = digit == '0' ? 'j' : static_cast<char>('a' + digit - '1');
								cipher += plain_to_braille.at(std::string(1, letter));
							} i++;
						} continue;
					} std::string match = find_plain(plain, i);
					if (match.empty()) {
						cipher.push_back(plain[i++]);
						continue;
					} bool uppercase = std::isupper(static_cast<unsigned char>(plain[i])) != 0;
					if (uppercase) cipher += braille::d2b("6");
					cipher += plain_to_braille.at(match);
					i += match.size();
				} return cipher;
			}

			inline std::string decode (const std::string& cipher) const {
				std::string plain;
				bool number = false;
				bool capital = false;
				for (std::string::size_type i = 0; i < cipher.size();) {
					if (!detail::is_braille_at(cipher, i)) {
						plain.push_back(cipher[i++]);
						number = false;
						continue;
					} const std::string symbol = cipher.substr(i, 3);
					i += 3;
					if (symbol == braille::d2b("3456")) {
						number = true;
						continue;
					} if (symbol == braille::d2b("6")) {
						capital = true;
						continue;
					} const auto found = braille_to_plain.find(symbol);
					if (found == braille_to_plain.end()) {
						plain += symbol;
						continue;
					} std::string value = found->second;
					if (number && value.size() == 1 && value[0] >= 'a' && value[0] <= 'j') {
						plain.push_back(value[0] == 'j' ? '0' : static_cast<char>('1' + value[0] - 'a'));
					} else {
						if (capital && !value.empty()) value[0] = static_cast<char>(std::toupper(static_cast<unsigned char>(value[0])));
						plain += value;
					} capital = false;
					number = number && ((value.size() == 1 && value[0] >= 'a' && value[0] <= 'j') ||
						is_number_character(value[0]));
				} return plain;
			}

			inline std::string encode () const { return encode(target); }
			inline std::string decode () const { return decode(target); }
	};
}

#endif
