# metadata 语法

`/metadata.json` 用来配置文件批注和本地化类型显示。

在仓库根目录创建 `metadata.json`，结构如下：

```json
{
  "files": {
    "braille.h": {
      "comment": "盲文处理头文件"
    },
    "test_case/test_file.txt": {
      "comment": "测试用文本文件"
    }
  },
  "locale": {
    "types": {
      ".pdf": "PDF文档",
      ".txt": "文本文件",
      ".exe": "可执行文件",
      ".zip": "压缩包"
    }
  }
}
```

## 配置项说明

### files
文件批注配置，以文件相对于 `files/` 的路径作为键。每个文件可以包含：
- `comment`: 文件说明文本，显示在索引的"说明"列中

### locale
本地化配置，包含：
- `types`: 文件类型映射表，将扩展名映射到友好的显示名称（如 `.pdf` -> "PDF文档"）

## 使用方法

填写或修改批注后，运行：

```bash
python3 generate_index.py
```

批注会显示在索引的"说明"列中。没有批注的文件会显示 `-`。类型显示会优先使用 `locale.types` 中的映射，如果没有匹配则使用 MIME 类型推断。

路径使用 `/`，文件名中的特殊字符可以直接写入 JSON。