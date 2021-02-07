#include <iostream>
#include <fstream>
#include <vector>
#include <array>

typedef uint32_t DWORD;


typedef uint8_t WORD;
typedef std::array<uint8_t, 4> Fourcc;


#pragma pack(push, r1, 1)

struct FOURCC {
    WORD c_1;
    WORD c_2;
    WORD c_3;
    WORD c_4;
};

typedef struct _avimainheader {
    FOURCC fcc;
    DWORD cb;
    DWORD dwMicroSecPerFrame;
    DWORD dwMaxBytesPerSec;
    DWORD dwPaddingGranularity;
    DWORD dwFlags;
    DWORD dwTotalFrames;
    DWORD dwInitialFrames;
    DWORD dwStreams;
    DWORD dwSuggestedBufferSize;
    DWORD dwWidth;
    DWORD dwHeight;
    DWORD dwReserved[4];
} AVIMAINHEADER;

typedef struct {
    FOURCC fccType;
    FOURCC fccHandler;
    DWORD dwFlags;
    WORD wPriority;
    WORD wLanguage;
    DWORD dwInitialFrames;
    DWORD dwScale;
    DWORD dwRate;
    DWORD dwStart;
    DWORD dwLength;
    DWORD dwSuggestedBufferSize;
    DWORD dwQuality;
    DWORD dwSampleSize;
    //RECT rcFrame;
} AVIStreamHeader;

const Fourcc LIST = {'L', 'I', 'S', 'T'};

//struct __attribute__ ((packed)) Chunk {

struct Chunk {
    Fourcc id;
    DWORD size;
    std::vector<char> data;
};

struct List {
    Fourcc name;
    DWORD size;
    Fourcc type;
    std::vector<char> data;
};


struct Avi {
    DWORD size;
    WORD filetype[4];
    // data Chunk | List
};

//struct __attribute__ ((packed)) File {
struct File {
    WORD name_1;
    WORD name_2;
    WORD name_3;
    WORD name_4;
    DWORD size;
    WORD filetype_1;
    WORD filetype_2;
    WORD filetype_3;
    WORD filetype_4;
};


#pragma pack(pop, r1)


void show_list(List l) {
    std::cout << "( " << l.name.data() << " size: " << l.size << " type: " << l.type.data() << ")" << std::endl;
}

void show_chunk(Chunk c) {
    std::cout << "( Chunk id: " << c.id.data() << " size: " << c.size << ")" << std::endl;
}

File read_header(std::fstream &file) {
    File f;
    file.read((char *) &f, sizeof(f));
    return f;
}


Fourcc read_fourcc(std::fstream &file) {
    FOURCC f;
    file.read((char *) &f, sizeof(f));
    Fourcc arr;
    arr = {f.c_1, f.c_2, f.c_3, f.c_4};
    return arr;
};

DWORD read_dword(std::fstream &file) {
    DWORD f;
    file.read((char *) &f, sizeof(f));
    return f;
}

Chunk read_chunk_header(std::fstream &file, Fourcc name) {
    Chunk c;
    c.id = name;
    c.size = read_dword(file);
    return c;
};

List read_list_header(std::fstream &file, Fourcc name) {
    List f;
    f.name = name;
    f.size = read_dword(file);
    f.type = read_fourcc(file);
    return f;
};

void read_stream_header(std::fstream &file) {
    AVIStreamHeader a;
    file.read((char *) &a, sizeof(a));
}

void read_format_header(std::fstream &file) {

}

void read_list(std::fstream &file, Fourcc name);

void read_chunk(std::fstream &file, Fourcc name);


const Fourcc STRF = {'s', 't', 'r', 'f'};
const Fourcc STRH = {'s', 't', 'r', 'h'};
const Fourcc AVIH = {'a', 'v', 'i', 'h'};

void skip(std::fstream &file, size_t size) {
    char znak;
    for (int i = 0; i < size; i++) {
        file.read((char *) &znak, sizeof(znak));
    }
}

void dispatch(std::fstream &file, Fourcc fourcc) {
    if (fourcc == LIST) {
        read_list(file, fourcc);
    } else {
        if (fourcc == STRH) {
            auto size = read_chunk_header(file, fourcc).size;
            skip(file, size);
            //read_stream_header(file);
        } else if (fourcc == STRF) {
            auto size = read_chunk_header(file, fourcc).size;
            skip(file, size);
            //read_format_header(file);
        } else if (fourcc == AVIH) {
            auto size = read_chunk_header(file, fourcc).size;
            skip(file, size);
        } else {
            auto size = read_chunk_header(file, fourcc).size;
            skip(file, size);
            //read_chunk(file, fourcc);
        }
    }
}

void read_list(std::fstream &file, Fourcc fourcc) {
    auto header = read_list_header(file, fourcc);
    DWORD position = 4;
    show_list(header);
    while (position < header.size) {
        auto fourcc = read_fourcc(file);
        dispatch(file, fourcc);
        position++;
    }
}


void read_chunk(std::fstream &file, Fourcc fourcc) {
    auto header = read_chunk_header(file, fourcc);
    DWORD position = 4;
    show_chunk(header);
    while (position < header.size) {
        auto fourcc = read_fourcc(file);
        dispatch(file, fourcc);
        position++;
    }
}


void read_avi() {

    Avi avi;

    std::fstream file;
    file.open("./drop.avi", std::ios::in | std::ios::binary);
    {
        // read header
        auto f = read_header(file);
        avi.size = f.size;
        avi.filetype[0] = f.filetype_1;
        avi.filetype[1] = f.filetype_2;
        avi.filetype[2] = f.filetype_3;
        avi.filetype[3] = f.filetype_4;
    }
    // jezeli filetype nie avi to wywal

    // poniewaz rozmiar podany
    // jest lacznie z rozszerzeniem ktore ma 4 bajty

    // problemem jest fakt ze nie przesowamy go ruszajac sie w srodku -
    // wystarczy dac ref na idx ale juz nie dzisiaj
    DWORD position = 4;
    while (position < 5) {
        auto fourcc = read_fourcc(file);
        if (fourcc == LIST) {
            read_list(file, fourcc);
        } else {
            read_chunk(file, fourcc);
        }
        position++;
    }

}

int main() {
    std::fstream file;

    std::cout << sizeof(File) << std::endl;

    file.open("./drop.avi", std::ios::in | std::ios::binary);

    read_avi();
}