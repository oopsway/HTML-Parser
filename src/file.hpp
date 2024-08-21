#include <cstdint>
#include <cstdio>

struct File
{
    File() = default;

    ~File()
    {
        if (_data)
            delete[] _data;
    }

    bool readAll(const char *filename)
    {
        FILE *f;
        if (fopen_s(&f, filename, "rb") != 0)
            return false;
        fseek(f, 0, SEEK_END);
        _size = ftell(f);
        fseek(f, 0, SEEK_SET);
        _data = new uint8_t[_size + 1];
        _data[_size] = '\0';
        fread(_data, _size, 1, f);
        fclose(f);
        return true;
    }
    
    void clear()
    {
        if (_data)
        {
            delete[] _data;
            _data = nullptr;
            _size = 0;
        }
    }

    size_t size() const
    {
        return _size;
    }

    uint8_t* data()
    {
        return _data;
    }

    const char* c_str()
    {
        if(!_data)
            return "";
        return (const char*)_data;
    }

private:
    uint8_t *_data = nullptr;
    size_t _size = 0;
};