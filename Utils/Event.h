#pragma once
#include "Types.h"
#include <list>
#include <functional>

namespace Phoenix
{
    template <typename... Args>
    struct Event
    {
    public:
        struct Callback
        {
            template <typename T>
            Callback(void (T::* func)(Args...), T* obj)
            {
                //Complicated conversion to store function ID
                union Converter
                {
                    void (T::* in)(Args...);
                    void* out;
                };
                Converter conversion;
                conversion.in = func;
                //end

                mID.mObject = obj;
                mID.mMethod = conversion.out;

                //This is key: Store the method as a closure
                mCallback = [=](Args... args)
                {
                    //execute the method on the object
                    (obj->*func)(args...);
                };
            }

            Callback(void (*func)(Args...))
            {
                mID.mObject = nullptr;
                mID.mMethod = func;

                //This is key: Store the method as a closure
                mCallback = [=](Args... args)
                {
                    //call the method
                    func(args...);
                };
            }

            bool operator==(const Callback& rhs)const
            {
                return ((mID.mObject == rhs.mID.mObject) &&
                    (mID.mMethod == rhs.mID.mMethod));
            }

            std::function<void(Args...)> mCallback;
            struct
            {
                void* mObject;
                void* mMethod;
            }mID;
        };

        void operator+=(const Callback& call)
        {
            auto found = std::find(mCallbacks.begin(), mCallbacks.end(), call);
            if (found == mCallbacks.end())
                mCallbacks.push_back(std::move(call));
        }
        void operator-=(const Callback& call)
        {
            mCallbacks.remove(call);
        }

        void operator()(Args... args)const
        {
            auto iter = mCallbacks.begin();
            while (iter != mCallbacks.end())
            {
                iter->mCallback(args...);
                ++iter;
            }
        }

    private:
        std::list<Callback> mCallbacks;
    };
}
