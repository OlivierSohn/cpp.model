#pragma once

#ifdef _WIN32
#define NOMINMAX
#include "Windows.h"
#include "Objbase.h"
#elif __ANDROID__
#else
#include <uuid/uuid.h>
#endif

#include <algorithm>
#include <array>
#include <cstddef>
#include <functional>
#include <list>
#include <limits>
#include <map>
#include <memory>
#include <stack>
#include <tuple>
#include <type_traits>
#include <vector>

#include "../../algorithms/include/public.h"
#include "../../os.log/include/public.h"
#include "../../os.storage/include/public.h"

#include "meta.h"
#include "raii.hpp"
#include "ref_unique_ptr.h"
#include "intrusive_ptr_defs.h"
#include "visitor.h"
#include "available_indexes.h"
#include "observable.h"
#include "visitable.h"
#include "weak_ptr.h"
#include "updatable.h"
#include "persistable.h"
#include "execution.h"
#include "history.manager.h"
#include "undoable.h"
#include "undoable.hpp"
#include "command.hpp"
#include "asyncnotifier.h"
#include "referentiable.h"
#include "intrusive_ptr.h"
namespace imajuscule {
    intrusive_ptr<Referentiable> instantiate(ReferentiableManagerBase * rm, const std::string & hintName);
    intrusive_ptr<Referentiable> instantiate(ReferentiableManagerBase * rm);
}
#include "globals.h"
#include "referentiable.manager.h"
#include "referentiables.h"
#include "referentiable.root.h"
#include "referentiable.cmd.list.h"
#include "referentiable.cmd.list.hpp"
#include "managed.reflist.hpp"
#include "referentiable.cmd.set.h"
#include "referentiable.cmd.set.hpp"
#include "referentiable.cmd.simpleset.h"
#include "referentiable.cmd.simpleset.hpp"
#include "application.h"