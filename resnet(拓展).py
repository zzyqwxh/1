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
# ===== 【改动：删除原图片标注保存逻辑，新增置信度排行功能】 =====
for img_path in img_paths:
    img = Image.open(img_path).convert("RGB")
    img_tensor = transform(img)
    img_tensor = img_tensor.unsqueeze(0)
    
    with torch.no_grad():
        output = model(img_tensor)
        # ===== 【新增1：softmax 转换为 0~100% 置信度概率】 =====
        probs = torch.softmax(output, dim=1)
        # ===== 【新增2：提取置信度最高的前5类，自动从高到低排序】 =====
        top_k = 5
        top_probs, top_indices = torch.topk(probs, k=top_k, dim=1)
        # 转为Python列表方便遍历
        top_indices = top_indices[0].tolist()
        top_probs = top_probs[0].tolist()

    # ===== 【新增3：拼接排行文本】 =====
    rank_lines = []
    for i in range(top_k):
        cls_name = labels[top_indices[i]]
        conf = top_probs[i] * 100
        rank_lines.append(f"Top{i+1}: {cls_name} ({conf:.2f}%)")

    # ===== 【改动：输出改为置信度排行格式】 =====
    print(f"{img_path} --> 置信度排行:")
    for line in rank_lines:
        print(f"  {line}")
    print()

# ====================End===================