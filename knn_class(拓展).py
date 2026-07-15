#  请在此处编写代码，实现使用 KNN 算法分类
# ====================Begin===================
# 导入必要的库
import pandas as pd
import time
from sklearn.model_selection import train_test_split
from sklearn.preprocessing import LabelEncoder, StandardScaler
from sklearn.neighbors import KNeighborsClassifier
from sklearn.linear_model import LogisticRegression
from sklearn.metrics import accuracy_score, precision_score, recall_score, f1_score


def run_knn_person_classification(train_size, random_state):
    # ==================== 1. 数据加载 ====================
    df = pd.read_csv("500_Person_Gender_Height_Weight_Index.csv")

    # ==================== 2. 生成自变量和因变量 ====================
    X = df[["Gender", "Height", "Weight"]].copy()
    y = df["Index"]

    # ==================== 3. 将 Gender 编码为数值 ====================
    le = LabelEncoder()
    X["Gender"] = le.fit_transform(X["Gender"])

    # ==================== 4. 数据集划分 ====================
    X_train, X_test, y_train, y_test = train_test_split(
        X, y,
        train_size=train_size,
        random_state=random_state,
        stratify=y
    )

    # ==================== 5. 特征标准化 ====================
    scaler = StandardScaler()
    X_train_scaled = scaler.fit_transform(X_train)
    X_test_scaled = scaler.transform(X_test)

    # ==================== 6. KNN 模型训练与评估（含计时） ====================
    # KNN训练计时
    start_knn_train = time.time()
    model = KNeighborsClassifier(n_neighbors=5, weights="distance", metric="euclidean")
    model.fit(X_train_scaled, y_train)
    end_knn_train = time.time()
    knn_train_time = end_knn_train - start_knn_train

    # KNN预测计时
    start_knn_pred = time.time()
    y_pred = model.predict(X_test_scaled)
    end_knn_pred = time.time()
    knn_pred_time = end_knn_pred - start_knn_pred

    accuracy = accuracy_score(y_test, y_pred)
    precision = precision_score(y_test, y_pred, average="weighted")
    recall = recall_score(y_test, y_pred, average="weighted")
    f1 = f1_score(y_test, y_pred, average="weighted")

    # ==================== 7. 逻辑回归对比实验（含计时） ====================
    # 逻辑回归训练计时
    start_lr_train = time.time()
    lr_model = LogisticRegression(max_iter=2000)
    lr_model.fit(X_train_scaled, y_train)
    end_lr_train = time.time()
    lr_train_time = end_lr_train - start_lr_train

    # 逻辑回归预测计时
    start_lr_pred = time.time()
    lr_pred = lr_model.predict(X_test_scaled)
    end_lr_pred = time.time()
    lr_pred_time = end_lr_pred - start_lr_pred

    lr_acc = accuracy_score(y_test, lr_pred)
    lr_pre = precision_score(y_test, lr_pred, average="weighted")
    lr_rec = recall_score(y_test, lr_pred, average="weighted")
    lr_f1 = f1_score(y_test, lr_pred, average="weighted")

    # ==================== 控制台打印全部结果 ====================
    print("===== KNN 模型评估结果 =====")
    print(f"准确率: {accuracy:.4f}")
    print(f"精确率: {precision:.4f}")
    print(f"召回率: {recall:.4f}")
    print(f"F1 分数: {f1:.4f}")

    print("\n===== 逻辑回归模型评估结果 =====")
    print(f"准确率: {lr_acc:.4f}")
    print(f"精确率: {lr_pre:.4f}")
    print(f"召回率: {lr_rec:.4f}")
    print(f"F1 分数: {lr_f1:.4f}")

    print("\n===== 运行耗时对比 =====")
    print(f"KNN 训练耗时: {knn_train_time:.4f} 秒")
    print(f"KNN 预测耗时: {knn_pred_time:.4f} 秒")
    print(f"逻辑回归 训练耗时: {lr_train_time:.4f} 秒")
    print(f"逻辑回归 预测耗时: {lr_pred_time:.4f} 秒")

    # 返回值保持原格式，完全适配自动评测脚本
    return float(accuracy), float(precision), float(recall), float(f1)
# ====================End===================

# 本地测试入口
if __name__ == "__main__":
    acc, pre, rec, f1 = run_knn_person_classification(0.7, 42)
    print(f"\nKNN 最终评测得分：{f1 * 100:.2f} 分")
