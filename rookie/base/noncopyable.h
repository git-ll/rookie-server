


#ifndef ROOKIE_NONCOPYABLE_H
#define ROOKIE_NONCOPYABLE_H



namespace rookie
{
    class noncopyable
    {
    public:
        noncopyable(const noncopyable&) = delete;
        void operator=(const noncopyable&) = delete;

        noncopyable() = default;
        ~noncopyable() = default;
    };
}

#endif
