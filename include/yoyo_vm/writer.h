#pragma once
namespace Yvm
{
    struct Writer
    {
        std::vector<uint64_t> data;
        size_t byte_off = 0;
        void write_byte(uint8_t byte)
        {
            if (byte_off >= data.size() * 8) data.push_back(0);
            reinterpret_cast<uint8_t*>(data.data())[byte_off++] = byte;
        }
        void write_opcode(OpCode opcode) {  write_byte(static_cast<uint8_t>(opcode));  }
        template<typename T>
        void write_n(T value)
        {
            constexpr auto n = sizeof(T);
            byte_off += (n - byte_off % n) % n;
            if (byte_off >= data.size() * 8) data.push_back(0);
            reinterpret_cast<T*>(data.data())[byte_off / n] = value;
            byte_off += n;
        }
    };
}