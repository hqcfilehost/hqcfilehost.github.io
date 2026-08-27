# metadata 语法

`/metadata.json` 目前主要用来写文件说明

在仓库根目录创建 `metadata.json`，用文件相对于 `files/` 的路径作为键：

```json
{
  "braille.h": {
    "comment": "盲文处理头文件"
  },
  "test_case/test_file.txt": {
    "comment": "测试用文本文件"
  }
}
```

填写或修改批注后，运行：

```bash
python3 generate_index.py
```

批注会显示在索引的“说明”列中。没有批注的文件会显示 `-`。路径使用 `/`，文件名中的特殊字符可以直接写入 JSON。
