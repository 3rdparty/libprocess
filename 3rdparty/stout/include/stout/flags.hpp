#ifndef __STOUT_FLAGS_HPP__
#define __STOUT_FLAGS_HPP__

#include <stout/flags/flags.hpp>

// An abstraction for application/library "flags". An example is
// probably best:
//  -------------------------------------------------------------
// class MyFlags : public virtual FlagsBase // Use 'virtual' for composition!
// {
// public:
//   Flags()
//   {
//     add(&debug,
//         "debug",
//         "Help string for debug",
//         false);
//
//     add(&name,
//         "name",
//         "Help string for name");
//   }

//   bool debug;
//   Option<string> name;
// };
//
// ...
//
// map<string, Option<string> > values;
// values["no-debug"] = None();                       // --no-debug
// values["debug"] = None();                          // --debug
// values["debug"] = Option<string>::some("true");    // --debug=true
// values["debug"] = Option<string>::some("false");   // --debug=false
// values["name"] = Option<string>::some("frank");    // --name=frank
//
// MyFlags flags;
// flags.load(values);
// flags.name.isSome() ...
// flags.debug ...
//  -------------------------------------------------------------
//
// You can also compose flags provided that each has used "virtual
// inheritance":
//  -------------------------------------------------------------
// Flags<MyFlags1, MyFlags2> flags;
// flags.add(...); // Any other flags you want to throw in there.
// flags.load(values);
// flags.flag_from_myflags1 ...
// flags.flag_from_myflags2 ...
//  -------------------------------------------------------------
//
// "Fail early, fail often":
//
// You can not add duplicate flags, this is checked for you at compile
// time for composite flags (e.g., Flag<MyFlags1, MyFlags2>) and also
// checked at runtime for any other flags added via inheritance or
// Flags::add(...).
//
// Flags that can not be loaded (e.g., attempting to use the 'no-'
// prefix for a flag that is not boolean) will print a message to
// standard error and abort the process.

// TODO(benh): Provide a boolean which specifies whether or not to
// abort on duplicates or load errors.

// TODO(benh): Make prefix for environment variables configurable
// (e.g., "MESOS_").

#endif // __STOUT_FLAGS_HPP__
