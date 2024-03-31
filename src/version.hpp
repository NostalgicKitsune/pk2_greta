//#########################
//Pekka Kana 2
//Copyright (c) 2003 Janne Kivilahti
//#########################
#pragma once

#define PK2_NAME "Pekka Kana 2"
#define PK2_VERSION_NAME "Greta Engine"


#ifndef PK2_VERSION
#define PK2_VERSION "April Fools 2024"
#endif


#ifdef PK2_USE_ZIP
#define PK2_ZIP_STR "(zip)"
#else
#define PK2_ZIP_STR "(no-zip)"
#endif

#ifdef PK2_USE_JAVA
#define PK2_JAVA_STR "(java)"
#else
#define PK2_JAVA_STR "(no-java)"
#endif


#define PK2_VERSION_STR PK2_NAME " " PK2_VERSION_NAME " " PK2_VERSION " " PK2_ZIP_STR " " PK2_JAVA_STR
#define PK2_VERSION_STR_MENU "Greta " PK2_VERSION