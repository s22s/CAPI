/*
 * Copyright (c) Simverge Software LLC - All Rights Reserved
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include <pdal/pdalc_config.h>
#include <pdal/gitsha.h>
#include <pdal/pdal_features.hpp>

#include "greatest.h"

SUITE(test_pdalc_config);

TEST testPDALVersionInfo(void)
{
	int versionInteger = PDALVersionInteger();
	ASSERT_EQ(PDAL_VERSION_INTEGER, versionInteger);

	int major = PDALVersionMajor();
	ASSERT_EQ(PDAL_VERSION_MAJOR, major);

	int minor = PDALVersionMinor();
	ASSERT_EQ(PDAL_VERSION_MINOR, minor);

	int patch = PDALVersionPatch();
	ASSERT_EQ(PDAL_VERSION_PATCH, patch);

	ASSERT_EQ(major*10000 + minor*100 + patch, versionInteger);

	char expected[64];
	sprintf(expected, "%d.%d.%d", major, minor, patch);

	char version[64];
	size_t size = PDALVersionString(version, 64);
	ASSERT(size > 0 && size <= 64);
	ASSERT(version[0]);
	ASSERT_STR_EQ(expected, version);
	ASSERT_STR_EQ(PDAL_VERSION_STRING, version);

	char sha1[64];
	size = PDALSha1(sha1, 64);
	ASSERT(size > 0 && size <= 64);
	ASSERT(sha1[0]);

	// Shorten SHA1 to six characters
	ASSERT(size > 6);
	sha1[6] = '\0';

	sprintf(expected + strlen(version), " (git-version: %s)", sha1);

	char fullVersion[64];
	size = PDALFullVersionString(fullVersion, 64);
	ASSERT(size > 0 && size <= 64);
	ASSERT(fullVersion[0]);
	ASSERT_STR_EQ(expected, fullVersion);

	PASS();
}

TEST testPDALDebugInformation(void)
{
	char info[1024];
	size_t size = PDALDebugInformation(info, 1024);
	ASSERT(size > 0 && size <= 1024);
	ASSERT(info[0]);
	PASS();
}

TEST testPDALPluginInstallPath(void)
{
	char path[1024];
	size_t size = PDALPluginInstallPath(path, 1024);
	ASSERT(size > 0 && size <= 1024);
	ASSERT(path[0]);

#if !defined USING_VCPKG_DEBUG
	ASSERT_STR_EQ(PDAL_PLUGIN_INSTALL_PATH, path);
#endif

	PASS();
}


GREATEST_SUITE(test_pdalc_config)
{
	RUN_TEST(testPDALVersionInfo);
	RUN_TEST(testPDALDebugInformation);
	RUN_TEST(testPDALPluginInstallPath);
}