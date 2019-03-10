cmake_minimum_required(VERSION 3.1)

# 加载框架
include(cmake/emake/import.cmake)

# 基本配置
EMakeProjectM(etools 1 0 0 0)
EMakeSetLogLevelF(2)                     # 0: 关闭，1：dbg，2：inf，3：wrn
EMakeSetBuildTypeM(Release)


EBuildAddExtM(base64   "" ON )
EBuildAddExtM(cnats    "" ON )
#EBuildAddExtM(pthread  "" OFF)    # pthread in win32
EBuildAddExtM(libuv    "" ON CREATE)

EBuildAddLibM(etools "" ON)

EBuildAddAppM(libuv_tester "" OFF)

EBuildAddAppM(tester "" ON CREATE)

# ------------------------------------
# -- 生成所需 kit 构建规则
EBuildGenerateM()


