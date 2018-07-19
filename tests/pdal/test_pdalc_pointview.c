/*
 * Copyright (c) Simverge Software LLC - All Rights Reserved
 */

#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#include "greatest.h"

#include <pdal/pdalc.h>

/// A simple binary tree implementation for int values
struct node
{
	int key;
	struct node *left;
	struct node *right;
};

void dispose(struct node *n)
{
	if (n != NULL)
	{
		dispose(n->left);
		dispose(n->right);
		free(n);
	}
}

bool insert(int key, struct node **n)
{
	bool inserted = false;

	if (*n == NULL)
	{
		*n = (struct node *) malloc(sizeof(struct node));
		(*n)->key = key;
		(*n)->left = NULL;
		(*n)->right = NULL;
		inserted = true;
	}
	else if (key < (*n)->key)
	{
		inserted = insert(key, &(*n)->left);
	}
	else if (key > (*n)->key)
	{
		inserted = insert(key, &(*n)->right);
	}

	return inserted;
}

SUITE(test_pdalc_pointview);

static const int INVALID_POINT_VIEW_ID = 0;
static PDALPipelinePtr gPipeline = NULL;
static PDALPointViewIteratorPtr gPointViewIterator = NULL;

static void setup_test_pdalc_pointview(void *arg)
{
	FILE *file = fopen("C:/workspace/nublar/pdal-c/build/x64-windows/data/simple-reproject.json", "rb");
	char *json = NULL;

	if (file)
	{
		fseek(file, 0, SEEK_END);
		long length = ftell(file);
		fseek(file, 0, SEEK_SET);
		char *json = malloc(length + 1);

		if (json)
		{
			fread(json, 1, length, file);
			json[length] = '\0';
			gPipeline = PDALCreatePipeline(json);

			if (gPipeline && PDALExecutePipeline(gPipeline))
			{
				gPointViewIterator = PDALGetPointViews(gPipeline);
			}

			free(json);
		}

		fclose(file);
	}
}

static void teardown_test_pdalc_pointview(void *arg)
{
	PDALDisposePointViewIterator(gPointViewIterator);
	PDALDisposePipeline(gPipeline);
}

TEST testPDALGetPointViewId(void)
{
	ASSERT_EQ(INVALID_POINT_VIEW_ID, PDALGetPointViewId(NULL));

	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	struct node *tree = NULL;

	while (hasNext)
	{
		PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
		ASSERT(view);

		// Make sure all IDs are valid
		int id = PDALGetPointViewId(view);
		ASSERT_FALSE(id == INVALID_POINT_VIEW_ID);

		// Make sure that there are no duplicate IDs
		bool inserted = insert(id, &tree);
		ASSERT(inserted);

		hasNext = PDALHasNextPointView(gPointViewIterator);
	}

	dispose(tree);

	PASS();
}

TEST testPDALGetPointViewSize(void)
{
	ASSERT_EQ(0, PDALGetPointViewSize(NULL));

	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	size_t size = PDALGetPointViewSize(view);
	ASSERT(size > 0);

	PDALDisposePointView(view);
	PASS();
}

TEST testPDALIsPointViewEmpty(void)
{
	ASSERT(PDALIsPointViewEmpty(NULL));

	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	ASSERT_FALSE(PDALIsPointViewEmpty(view));
	PDALDisposePointView(view);

	PASS();
}

TEST testPDALClonePointView(void)
{
	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	PDALPointLayoutPtr anotherView = PDALClonePointView(view);

	ASSERT(anotherView);
	ASSERT(view != anotherView);
	ASSERT(PDALGetPointViewId(view) != PDALGetPointViewId(anotherView));

	size_t capacity = 1024;
	char expected[1024];
	char actual[1024];

	size_t expectedLength = PDALGetPointViewProj4(view, expected, capacity);
	ASSERT(expectedLength > 0 && expectedLength <= capacity);
	ASSERT(expected[0] != '\0');

	size_t actualLength = PDALGetPointViewProj4(view, actual, capacity);
	ASSERT_EQ(expectedLength, actualLength);
	ASSERT_STR_EQ(expected, actual);

	expectedLength = PDALGetPointViewWkt(view, expected, capacity, false);
	ASSERT(expectedLength > 0 && expectedLength <= capacity);
	ASSERT(expected[0] != '\0');

	actualLength = PDALGetPointViewWkt(view, actual, capacity, false);
	ASSERT_EQ(expectedLength, actualLength);
	ASSERT_STR_EQ(expected, actual);

	expectedLength = PDALGetPointViewWkt(view, expected, capacity, true);
	ASSERT(expectedLength > 0 && expectedLength <= capacity);
	ASSERT(expected[0] != '\0');

	actualLength = PDALGetPointViewWkt(view, actual, capacity, true);
	ASSERT_EQ(expectedLength, actualLength);
	ASSERT_STR_EQ(expected, actual);

	PDALDisposePointView(anotherView);
	PDALDisposePointView(view);

	PASS();
}

TEST testPDALGetPointViewProj4(void)
{
	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	size_t capacity = 1024;
	char proj[1024];

	size_t size = PDALGetPointViewProj4(NULL, proj, capacity);
	ASSERT_EQ(0, size);
	ASSERT_EQ('\0', proj[0]);

	size = PDALGetPointViewProj4(view, NULL, capacity);
	ASSERT_EQ(0, size);

	size = PDALGetPointViewProj4(view, proj, 0);
	ASSERT_EQ(0, size);

	size = PDALGetPointViewProj4(view, proj, capacity);
	ASSERT(size > 0 && size <= capacity);
	ASSERT_FALSE(proj[0] == '\0');
	ASSERT_STR_EQ("+proj=longlat +datum=WGS84 +no_defs", proj);

	PDALDisposePointView(view);

	PASS();
}

TEST testPDALGetPointViewWkt(void)
{
	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	size_t capacity = 1024;
	char wkt[1024];

	size_t size = PDALGetPointViewWkt(NULL, wkt, capacity, false);
	ASSERT_EQ(0, size);
	ASSERT_EQ('\0', wkt[0]);

	size = PDALGetPointViewWkt(view, NULL, capacity, false);
	ASSERT_EQ(0, size);

	size = PDALGetPointViewWkt(view, wkt, 0, false);
	ASSERT_EQ(0, size);

	size = PDALGetPointViewWkt(view, wkt, capacity, false);
	ASSERT(size > 0 && size <= capacity);
	ASSERT_FALSE(wkt[0] == '\0');
	ASSERT_STR_EQ(
		"GEOGCS[\"WGS 84\","
			"DATUM[\"WGS_1984\","
				"SPHEROID[\"WGS 84\",6378137,298.257223563,AUTHORITY[\"EPSG\",\"7030\"]],"
				"AUTHORITY[\"EPSG\",\"6326\"]"
			"],"
			"PRIMEM[\"Greenwich\",0,AUTHORITY[\"EPSG\",\"8901\"]],"
			"UNIT[\"degree\",0.0174532925199433,AUTHORITY[\"EPSG\",\"9122\"]],"
			"AUTHORITY[\"EPSG\",\"4326\"]"
		"]",
		wkt
	);


	char prettyWkt[1024];
	size_t prettySize = PDALGetPointViewWkt(view, prettyWkt, capacity, true);
	ASSERT(prettySize > 0 && prettySize <= capacity);
	ASSERT(size < prettySize);
	ASSERT(strcmp(wkt, prettyWkt) != 0);

	PDALDisposePointView(view);

	PASS();
}


TEST testPDALGetPointViewLayout(void)
{
	PDALPointLayoutPtr layout = PDALGetPointViewLayout(NULL);
	ASSERT_EQ(NULL, layout);

	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	layout = PDALGetPointViewLayout(view);
	ASSERT(layout);

	PDALDisposePointView(view);

	PASS();
}

TEST testPDALGetPackedPoint(void)
{
	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	PDALPointLayoutPtr layout = PDALGetPointViewLayout(view);
	ASSERT(layout);
	PDALDimTypeListPtr dims = PDALGetPointLayoutDimTypes(layout);
	ASSERT(dims);

	uint64_t numPoints = PDALGetPointViewSize(view);
	size_t expected = PDALGetPointSize(layout);

	size_t capacity = 512;
	char buffer[512];
	ASSERT(expected > 0 && expected <= capacity);

	for (uint64_t i = 0; i < numPoints; ++i)
	{
		size_t actual = PDALGetPackedPoint(NULL, NULL, i, NULL);
		ASSERT_EQ(0, actual);
		actual = PDALGetPackedPoint(NULL, NULL, i, buffer);
		ASSERT_EQ(0, actual);
		actual = PDALGetPackedPoint(NULL, dims, i, NULL);
		ASSERT_EQ(0, actual);
		actual = PDALGetPackedPoint(NULL, dims, i, buffer);
		ASSERT_EQ(0, actual);
		actual = PDALGetPackedPoint(view, NULL, i, NULL);
		ASSERT_EQ(0, actual);
		actual = PDALGetPackedPoint(view, NULL, i, buffer);
		ASSERT_EQ(0, actual);
		actual = PDALGetPackedPoint(view, dims, i, NULL);
		ASSERT_EQ(0, actual);

		actual = PDALGetPackedPoint(view, dims, i, buffer);
		ASSERT_EQ(expected, actual);
	}

	PDALDisposePointView(view);

	PASS();
}

TEST testPDALGetAllPackedPoints(void)
{
	PDALResetPointViewIterator(gPointViewIterator);
	bool hasNext = PDALHasNextPointView(gPointViewIterator);
	ASSERT(hasNext);

	uint64_t actualSize = PDALGetAllPackedPoints(NULL, NULL, NULL);
	ASSERT_EQ(0, actualSize);

	PDALPointViewPtr view = PDALGetNextPointView(gPointViewIterator);
	ASSERT(view);

	actualSize = PDALGetAllPackedPoints(view, NULL, NULL);
	ASSERT_EQ(0, actualSize);

	PDALPointLayoutPtr layout = PDALGetPointViewLayout(view);
	ASSERT(layout);
	PDALDimTypeListPtr dims = PDALGetPointLayoutDimTypes(layout);
	ASSERT(dims);

	actualSize = PDALGetAllPackedPoints(NULL, dims, NULL);
	ASSERT_EQ(0, actualSize);
	actualSize = PDALGetAllPackedPoints(view, dims, NULL);
	ASSERT_EQ(0, actualSize);

	uint64_t numPoints = PDALGetPointViewSize(view);
	size_t pointSize = PDALGetPointSize(layout);
	ASSERT(numPoints > 0);
	ASSERT(pointSize > 0);

	char *actualPoints = calloc(numPoints, pointSize);
	ASSERT(actualPoints);

	actualSize = PDALGetAllPackedPoints(NULL, NULL, actualPoints);
	ASSERT_EQ(0, actualSize);
	actualSize = PDALGetAllPackedPoints(view, NULL, actualPoints);
	ASSERT_EQ(0, actualSize);
	actualSize = PDALGetAllPackedPoints(NULL, dims, actualPoints);
	ASSERT_EQ(0, actualSize);
	actualSize = PDALGetAllPackedPoints(view, dims, actualPoints);
	ASSERT_EQ(numPoints * pointSize, actualSize);

	char *expectedPoint = calloc(1, pointSize);
	ASSERT(expectedPoint);

	for (uint64_t i = 0; i < numPoints; ++i)
	{
		ASSERT_EQ(pointSize, PDALGetPackedPoint(view, dims, i, expectedPoint));
		ASSERT_MEM_EQ(expectedPoint, actualPoints + i * pointSize, pointSize);
	}

	free(expectedPoint);
	free(actualPoints);
	PDALDisposePointView(view);

	PASS();
}

GREATEST_SUITE(test_pdalc_pointview)
{
	SET_SETUP(setup_test_pdalc_pointview, NULL);
	SET_TEARDOWN(teardown_test_pdalc_pointview, NULL);

	RUN_TEST(testPDALGetPointViewId);
	RUN_TEST(testPDALGetPointViewSize);
	RUN_TEST(testPDALIsPointViewEmpty);
	RUN_TEST(testPDALClonePointView);
	RUN_TEST(testPDALGetPointViewProj4);
	RUN_TEST(testPDALGetPointViewWkt);
	RUN_TEST(testPDALGetPointViewLayout);
	RUN_TEST(testPDALGetPackedPoint);
	RUN_TEST(testPDALGetAllPackedPoints);

	SET_SETUP(NULL, NULL);
	SET_TEARDOWN(NULL, NULL);
}