#include <opencv2/core/core.hpp>
#include <opencv2/highgui/highgui.hpp>
#include <opencv2/imgproc/imgproc.hpp>

#include <vips/vips8>

int main(int argc, char **argv)
{
	cv::Mat img = cv::imread("E:\\Pathology\\splice\\2019-01-18-183425-273\\slide.jpg");
	if (img.empty())
		return -1;

	int width = img.cols;
	int height = img.rows;
	size_t length = 3 * width * height;
	void *data = img.data;

	if (VIPS_INIT(argv[0]))
		return -1;

	vips::VImage out = vips::VImage::new_from_memory(data, length,
		width, height, 3, VIPS_FORMAT_UCHAR);

	/* opencv is BGR, libvips is RGB, we must swap the first and last
	 * channels.
	 */
	std::vector<vips::VImage> bands = out.bandsplit();
	vips::VImage t = bands[0];
	bands[0] = bands[2];
	bands[2] = t;
	out = vips::VImage::bandjoin(bands);

	out.write_to_file("E:\\Pathology\\splice\\2019-01-18-183425-273\\pyramid.tif");

	return 0;
}