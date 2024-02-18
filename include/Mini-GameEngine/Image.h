#pragma once

#include <string>

#include <vulkan/vulkan.hpp>

namespace MiniGameEngine{
	enum class ImageFormat{
		None=0,
		RGBA,
		RGBA32F
	};

	class Image{
	public:
		Image(std::string_view path);
		Image(uint32_t width, uint32_t height, ImageFormat format, const void* data = nullptr);
		~Image();

		void setData(const void* data);

		VkDescriptorSet getDescriptorSet() const { return _descriptorSet; }

		void resize(uint32_t w, uint32_t h);

		uint32_t getWidth() const { return _width; }
		
		uint32_t getHeight() const { return _height; }

	private:
		void allocateMemory(uint64_t size);
		void release();

	private:
		uint32_t _width=0, _height = 0;
		VkImage _image = nullptr;
		VkImageView _imageView = nullptr;
		VkDeviceMemory _memory = nullptr;
		VkSampler _sampler = nullptr;

		ImageFormat _format=ImageFormat::None;

		VkBuffer _stagingBuffer = nullptr;
		VkDeviceMemory _stagingBufferMemory = nullptr;

		size_t _alignedSize = 0;

		VkDescriptorSet _descriptorSet = nullptr;

		std::string _filepath;
		

	};
};
