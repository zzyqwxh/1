# ====================Begin===================
import warnings
warnings.filterwarnings("ignore", message=".*pretrained.*", category=UserWarning)
warnings.filterwarnings("ignore", message=".*weights.*", category=UserWarning)
import torch
import os
import glob
from torchvision import transforms, models
from PIL import Image

# ==================== 1. 图片加载 ====================
# 图片路径
img_dir = "img"
img_paths = sorted(glob.glob(os.path.join(img_dir, "*")))

#  请在此处编写代码，实现使用预训练 ResNet18 模型对图像进行分类
# ====================Begin===================

# ==================== 2. 图像预处理 ====================
# 预训练模型 ResNet18 期望 224x224, RGB, 并进行标准化
transform = transforms.Compose([
    transforms.Resize((224, 224)),
    transforms.ToTensor(),
    transforms.Normalize(mean=[0.485, 0.456, 0.406], std=[0.229, 0.224, 0.225])
])

# ==================== 3. 模型加载 ====================
model = models.resnet18(pretrained=True)
model.eval()


# ==================== 4. ImageNet 标签映射====================
labels = []
with open("imagenet_classes.txt", "r", encoding="utf-8") as f:
    labels = [line.strip() for line in f.readlines()]

# ==================== 5. 批量图片推理 ====================
for img_path in img_paths:
    img = Image.open(img_path).convert("RGB")
    img_tensor = transform(img)
    img_tensor = img_tensor.unsqueeze(0)
    with torch.no_grad():
        output = model(img_tensor)
        pred_idx = torch.argmax(output, dim=1).item()
    pred_label = labels[pred_idx]


    print(f"{img_path} --> Predicted class: {pred_label}")

# ====================End===================

