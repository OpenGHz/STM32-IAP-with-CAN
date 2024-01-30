#!/usr/bin/env python

# https://blog.csdn.net/and_then111/article/details/86744938


class BinTools(object):
    @staticmethod
    def check_file(file_path):
        """
        检查文件是否存在
        :param file_path: 文件路径
        :return: True or False
        """
        return os.path.exists(file_path)

    @staticmethod
    def ensure_dir(dir_path):
        """
        确保目录存在
        :param dir_path: 目录路径
        :return: None
        """
        if not os.path.exists(dir_path):
            os.makedirs(dir_path)

    @staticmethod
    def read_bin_file(file_path, to_byte_list=False):
        """
        读取二进制文件
        :param file_path: 文件路径
        :return: 二进制数据
        """
        with open(file_path, "rb") as f:
            data = f.read()
        if to_byte_list:
            data = list(data)
        return data

    @staticmethod
    def get_file_size(file_path):
        """
        获取文件大小
        :param file_path: 文件路径
        :return: 文件大小（多少字节）
        """
        return os.path.getsize(file_path)

    @staticmethod
    def write_bin_file(file_path, data):
        """
        写入二进制文件
        :param file_path: 文件路径
        :param data: 二进制数据
        :return: None
        """
        with open(file_path, "wb") as f:
            f.write(data)


class Biner(object):
    def __init__(self, file_path) -> None:
        if not BinTools.check_file(file_path):
            raise Exception("file not exists")
        self.file_path = file_path
        self.file_size = BinTools.get_file_size(file_path)
        self.bytes_num = self.file_size
        self.data = BinTools.read_bin_file(file_path)


if __name__ == "1":
    import os

    filepath = "x.bin"
    binfile = open(filepath, "rb")  # 打开二进制文件
    size = os.path.getsize(filepath)  # 获得文件大小
    for i in range(size):
        data = binfile.read(1)  # 每次输出一个字节
        print(data)
    binfile.close()

    import struct

    for i in range(size):
        data = binfile.read(1)
        num = struct.unpack("B", data)
        print(num[0])

if __name__ == "__main__":
    a = 123
    print("a:", type(a))
    b_big = a.to_bytes(1, "big")
    print("b:", type(b_big))
    print(b_big)
    b_small = a.to_bytes(1, "little")
    print(b_small)

    data = 123
    content = data.to_bytes(1, "big")

    filepath = "123.bin"
    binfile = open(filepath, "ab+")  # 追加写入
    binfile.write(content)
    print("content", content)
    binfile.close()


"""
模式	描述
r	以只读方式打开文件。文件的指针将会放在文件的开头。这是默认模式。
rb	以二进制格式打开一个文件用于只读。文件指针将会放在文件的开头。这是默认模式。
r+	打开一个文件用于读写。文件指针将会放在文件的开头。
rb+	以二进制格式打开一个文件用于读写。文件指针将会放在文件的开头。
w	打开一个文件只用于写入。如果该文件已存在则将其覆盖。如果该文件不存在，创建新文件。
wb	以二进制格式打开一个文件只用于写入。如果该文件已存在则将其覆盖。如果该文件不存在，创建新文件。
w+	打开一个文件用于读写。如果该文件已存在则将其覆盖。如果该文件不存在，创建新文件。
wb+	以二进制格式打开一个文件用于读写。如果该文件已存在则将其覆盖。如果该文件不存在，创建新文件。
a	打开一个文件用于追加。如果该文件已存在，文件指针将会放在文件的结尾。也就是说，新的内容将会被写入到已有内容之后。如果该文件不存在，创建新文件进行写入。
ab	以二进制格式打开一个文件用于追加。如果该文件已存在，文件指针将会放在文件的结尾。也就是说，新的内容将会被写入到已有内容之后。如果该文件不存在，创建新文件进行写入。
a+	打开一个文件用于读写。如果该文件已存在，文件指针将会放在文件的结尾。文件打开时会是追加模式。如果该文件不存在，创建新文件用于读写。
ab+	以二进制格式打开一个文件用于追加。如果该文件已存在，文件指针将会放在文件的结尾。如果该文件不存在，创建新文件用于读写。
"""
