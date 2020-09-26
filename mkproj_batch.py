import os

ROOT_DIR = os.getcwd()
BUILD_DIR = os.path.join(ROOT_DIR, "build")
LIB_DIR = os.path.join(ROOT_DIR, "lib")
INC_DIR = os.path.join(ROOT_DIR, "include")
SRC_DIR = os.path.join(ROOT_DIR, "src")

lib_files = os.listdir(LIB_DIR)
src_files = os.listdir(SRC_DIR)

src_files.remove("main.cpp")
src_files.insert(0, "main.cpp")

src_args = " ".join(["..\\src\\" + file for file in src_files])
lib_args = " ".join(["..\\lib\\" + file for file in lib_files])

os.chdir(BUILD_DIR)
os.system("cl /EHsc /I ..\\include /c " + src_args)

obj_files = [file for file in os.listdir(BUILD_DIR) if file.endswith(".obj")]
obj_args = " ".join(obj_files)
os.system("link /OUT:..\\puppet.exe " + obj_args + " " + lib_args)
os.chdir(ROOT_DIR)

