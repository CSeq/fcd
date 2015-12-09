//
// metadata.h
// Copyright (C) 2015 Félix Cloutier.
// All Rights Reserved.
//
// This file is part of fcd.
// 
// fcd is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
// 
// fcd is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
// 
// You should have received a copy of the GNU General Public License
// along with fcd.  If not, see <http://www.gnu.org/licenses/>.
//

#ifndef fcd__metadata_h
#define fcd__metadata_h

#include "llvm_warnings.h"

SILENCE_LLVM_WARNINGS_BEGIN()
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/Metadata.h>
SILENCE_LLVM_WARNINGS_END()

#include <string>

namespace md
{
	llvm::ConstantInt* getVirtualAddress(const llvm::Function& fn);
	llvm::MDString* getImportName(const llvm::Function& fn);
	bool hasRecoveredArguments(const llvm::Function& fn);
	bool isPrototype(const llvm::Function& fn);
	
	void setVirtualAddress(llvm::Function& fn, uint64_t virtualAddress);
	void setImportName(llvm::Function& fn, llvm::StringRef name);
	void setRecoveredArguments(llvm::Function& fn);
	void setPrototype(llvm::Function& fn, bool prototype = true);
	
	void copy(const llvm::Function& from, llvm::Function& to);
	
	bool isRegisterStruct(const llvm::Value& value);
	void setRegisterStruct(llvm::AllocaInst& alloca, bool registerStruct = true);
}

#endif /* fcd__metadata_h */