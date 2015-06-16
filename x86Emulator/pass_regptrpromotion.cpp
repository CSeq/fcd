//
//  pass_regptrpromote.cpp
//  x86Emulator
//
//  Created by Félix on 2015-06-15.
//  Copyright © 2015 Félix Cloutier. All rights reserved.
//

#include "llvm_warnings.h"

SILENCE_LLVM_WARNINGS_BEGIN()
#include <llvm/Analysis/AliasAnalysis.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/Support/raw_ostream.h>
SILENCE_LLVM_WARNINGS_END()

#include "passes.h"

using namespace llvm;
using namespace std;

namespace
{
	// This pass is a little bit of a hack.
	// Emulators create weird code for union access. Bitcasts that target just part of a register use a GEP to
	// the struct that encloses the i64. The address is the same, but the type is different, and this angers
	// argument promotion. This pass fixes the GEPs to always use the i64 pointer.
	struct RegisterPointerPromotion : public FunctionPass
	{
		static char ID;
		
		RegisterPointerPromotion() : FunctionPass(ID)
		{
		}
		
		virtual void getAnalysisUsage(AnalysisUsage& au) const override
		{
			au.addRequired<AliasAnalysis>();
		}
		
		virtual bool runOnFunction(Function& f) override
		{
			assert(f.arg_size() == 1);
			bool modified = false;
			
			// Copy arguments to independent list to avoid iterating while modifying.
			Argument* firstArg = f.arg_begin();
			SmallVector<User*, 16> users(firstArg->user_begin(), firstArg->user_end());
			for (auto user : users)
			{
				if (auto gep = dyn_cast<GetElementPtrInst>(user))
				{
					if (isa<StructType>(gep->getResultElementType()))
					{
						fixGep(*gep);
						modified = true;
					}
				}
			}
			return modified;
		}
		
		void fixGep(GetElementPtrInst& gep)
		{
			LLVMContext& ctx = gep.getContext();
			SmallVector<Value*, 4> indices(gep.idx_begin(), gep.idx_end());
			indices.push_back(ConstantInt::get(Type::getInt32Ty(ctx), 0));
			GetElementPtrInst* goodGep = GetElementPtrInst::CreateInBounds(gep.getPointerOperand(), indices, "", &gep);
			
			auto& aa = getAnalysis<AliasAnalysis>();
			// We can't use replaceAllUsesWith because the type is different.
			while (!gep.use_empty())
			{
				auto badCast = cast<CastInst>(gep.user_back());
				auto goodCast = CastInst::Create(badCast->getOpcode(), goodGep, badCast->getType(), "", badCast);
				badCast->replaceAllUsesWith(goodCast);
				
				aa.replaceWithNewValue(badCast, goodCast);
				goodCast->takeName(badCast);
				badCast->eraseFromParent();
			}
			
			aa.replaceWithNewValue(&gep, goodGep);
			goodGep->takeName(&gep);
			gep.eraseFromParent();
		}
	};
	
	char RegisterPointerPromotion::ID = 0;
	RegisterPass<RegisterPointerPromotion> regPass("rptrp", "Register Pointer Promotion", false, false);
}

FunctionPass* createRegisterPointerPromotionPass()
{
	return new RegisterPointerPromotion;
}